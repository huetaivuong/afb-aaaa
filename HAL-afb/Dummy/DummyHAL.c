/*
 * Copyright (C) 2017 "Audiokinetic Inc"
 * Author Francois Thibault <fthibault@audiokinetic.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define _GNU_SOURCE
#include "hal-interface.h"
#include "audio-common.h"


// Define few private tag for not standard functions
//Old Softvolume
#define PCM_Volume_Multimedia 1000
#define PCM_Volume_Navigation 1001
#define PCM_Volume_Emergency  1002

// AK
#define PCM_Volume_Guidance 1003
#define PCM_Volume_Entertainment 1004
#define PCM_Volume_Notification  1005
#define PCM_Volume_Communication 1006
#define PCM_Volume_Warning 1007


// Default Values for MasterVolume Ramping
STATIC halVolRampT volRampMaster= {
    .mode    = RAMP_VOL_NORMAL,
    .slave   = Master_Playback_Volume,
    .delay   = 100*1000, // ramping delay in us
    .stepDown=1,
    .stepUp  =1,
};

STATIC halVolRampT volRampGuidance= {
    .slave   = Guidance_Vol_ID,
    .delay   = 50*1000, // ramping delay in us
    .stepDown= 2,
    .stepUp  = 4,
};

STATIC halVolRampT volRampEntertainment= {
    .slave   = Entertainment_Vol_ID,
    .delay   = 100*1000, // ramping delay in us
    .stepDown= 4,
    .stepUp  = 2,
};
// Default Values for MasterVolume Ramping
STATIC halVolRampT volRampNotification= {
    .slave   = Notification_Vol_ID,
    .delay   = 50*1000, // ramping delay in us
    .stepDown= 6,
    .stepUp  = 4,
};

STATIC halVolRampT volRampCommunication= {
    .slave   = Communication_Vol_ID,
    .delay   = 50*1000, // ramping delay in us
    .stepDown= 6,
    .stepUp  = 4,
};

STATIC halVolRampT volRampWarning= {
    .slave   = Warning_Vol_ID,
    .delay   = 20*1000, // ramping delay in us
    .stepDown= 6,
    .stepUp  = 10,
};

// Map HAL hight sndctl with Alsa numid and optionally with a custom callback for non Alsa supported functionalities.
STATIC alsaHalMapT  alsaHalMap[]= {
//   { .tag=Master_Playback_Volume, .ctl={.name="Master", .value=80 } },

  // Sound card does not have hardware volume ramping
  { .tag=Master_Playback_Ramp   , .cb={.callback=volumeRamp, .handle=&volRampMaster}, .info="ramp volume linearly according to current ramp setting",
    .ctl={.numid=CTL_AUTO, .type=SND_CTL_ELEM_TYPE_INTEGER, .count=1, .minval=0, .maxval=100, .step=1, .name="Master_Ramp"}
  },

  // Implement Rampup Volume for Virtual Channels (0-100)
  { .tag=Guidance_Vol_Ramp_ID, .cb={.callback=volumeRamp, .handle=&volRampGuidance}, .info="RampUp Guidance Volume",
    .ctl={.numid=CTL_AUTO, .type=SND_CTL_ELEM_TYPE_INTEGER,.name="Guidance_Ramp", .minval=0, .maxval=100, .step=1, .value=80 }
  },
  { .tag=Entertainment_Vol_Ramp_ID, .cb={.callback=volumeRamp, .handle=&volRampEntertainment}, .info="Rampup Entertainment Volume",
    .ctl={.numid=CTL_AUTO, .type=SND_CTL_ELEM_TYPE_INTEGER,.name="Entertainment_Ramp", .minval=0, .maxval=100, .step=1, .value=80 }
  },
  { .tag=Notification_Vol_Ramp_ID, .cb={.callback=volumeRamp, .handle=&volRampNotification}, .info="Ramp-up Notification Volume",
    .ctl={.numid=CTL_AUTO, .type=SND_CTL_ELEM_TYPE_INTEGER, .name="Notification_Ramp", .minval=0, .maxval=100, .step=1, .value=80 }
  },
  { .tag=Communication_Vol_Ramp_ID, .cb={.callback=volumeRamp, .handle=&volRampCommunication}, .info="RampUp Communication Volume",
    .ctl={.numid=CTL_AUTO, .type=SND_CTL_ELEM_TYPE_INTEGER,.name="Communication_Ramp", .minval=0, .maxval=100, .step=1, .value=80 }
  },
  { .tag=Warning_Vol_Ramp_ID, .cb={.callback=volumeRamp, .handle=&volRampWarning}, .info="Ramp-up Warning Volume",
    .ctl={.numid=CTL_AUTO, .type=SND_CTL_ELEM_TYPE_INTEGER, .name="Warning_Ramp", .minval=0, .maxval=100, .step=1, .value=80 }
  },

  // Bind with existing ones created by ALSA configuration (and linked to softvol) [0-255]
  { .tag=Guidance_Vol_ID     , .ctl={.name="Guidance_Vol", .minval=0, .maxval=255, .step=1, .value=204 } },
  { .tag=Entertainment_Vol_ID     , .ctl={.name="Entertainment_Vol", .minval=0, .maxval=255, .step=1, .value=204 } },
  { .tag=Notification_Vol_ID     , .ctl={.name="Notification_Vol", .minval=0, .maxval=255, .step=1, .value=204 } },
  { .tag=Communication_Vol_ID     , .ctl={.name="Communications_Vol", .minval=0, .maxval=255, .step=1, .value=204 } },
  { .tag=Warning_Vol_ID     , .ctl={.name="Warning_Vol", .minval=0, .maxval=255, .step=1, .value=204 } },

  // Could have similar controls for input  side

  { .tag=EndHalCrlTag}  /* marker for end of the array */
} ;

// HAL sound card mapping info
STATIC alsaHalSndCardT alsaHalSndCard = {
    .name = "Dummy", //  WARNING: name MUST match with 'aplay -l'
    .info = "Hardware Abstraction Layer for dummy sound card",
    .ctls = alsaHalMap,
};

STATIC int sndServiceInit() {
    int err;
    AFB_DEBUG("Dummy HAL Binding Init");

    err = halServiceInit(afbBindingV2.api, &alsaHalSndCard);
    return err;
}

// API prefix should be unique for each snd card
PUBLIC const struct afb_binding_v2 afbBindingV2 = {
    .api = "dummy",
    .init = sndServiceInit,
    .verbs = halServiceApi,
    .onevent = halServiceEvent,
};
