/*
 * Copyright (C) 2016 "IoT.bzh"
 * Author Fulup Ar Foll <fulup@iot.bzh>
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
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>

#include "AlsaCoreBinding.h"

PUBLIC const struct afb_binding_interface *afbIface;   

/*
 * array of the verbs exported to afb-daemon
 */
static const struct afb_verb_desc_v1 binding_verbs[] = {
  /* VERB'S NAME            SESSION MANAGEMENT          FUNCTION TO CALL         SHORT DESCRIPTION */
  { .name= "ping"   ,    .session= AFB_SESSION_NONE, .callback= pingtest,        .info= "Ping Binding" },
  { .name= "getinfo",    .session= AFB_SESSION_NONE, .callback= alsaGetInfo,     .info= "List All/One Sound Cards Info" },
  { .name= "getctls",    .session= AFB_SESSION_NONE,.callback= alsaGetCtls,      .info= "List All/One Controls from selected sndcard" },
  { .name= "subscribe",  .session= AFB_SESSION_NONE, .callback= alsaSubcribe,    .info= "Subscribe to events from selected sndcard" },
  { .name= "getcardid",  .session= AFB_SESSION_NONE, .callback= alsaGetCardId,   .info= "Get CardId from its short/long name" },
  { .name= "registerHal",.session= AFB_SESSION_NONE,.callback= alsaRegisterHal,  .info= "Register Hal CardName/ApiPrefix" },
  { .name= "ucmquery",   .session= AFB_SESSION_NONE, .callback= alsaUseCaseQuery,.info= "Use Case Query" },
  { .name= "ucmset",     .session= AFB_SESSION_NONE, .callback= alsaUseCaseSet,  .info= "Use Case Set" },
  { .name= "ucmget",     .session= AFB_SESSION_NONE, .callback= alsaUseCaseGet,  .info= "Use Case Get" },
  { .name= "ucmreset",   .session= AFB_SESSION_NONE, .callback= alsaUseCaseReset,.info= "Use Case Reset to Default" },
  { .name= "ucmclose",   .session= AFB_SESSION_NONE, .callback= alsaUseCaseClose,.info= "Use Case Close Manager" },
  { .name= NULL } /* marker for end of the array */
};

/*
 * description of the binding for afb-daemon
 */
static const struct afb_binding binding_description = {
  /* description conforms to VERSION 1 */
  .type= AFB_BINDING_VERSION_1,
  .v1= {
    .prefix= "alsacore",
    .info= "Low Level Interface to Alsa Sound Lib",
    .verbs = binding_verbs
  }
};

extern int afbBindingV1ServiceInit(struct afb_service service) {
   // this is call when after all bindings are loaded
   // alsaLibInit (service);  // AlsaBinding check for sound card at installation time   
   return (0); 
};

/*
 * activation function for registering the binding called by afb-daemon
 */
const struct afb_binding *afbBindingV1Register(const struct afb_binding_interface *itf) {
    afbIface= itf;
    
    return &binding_description;	/* returns the description of the binding */
}

