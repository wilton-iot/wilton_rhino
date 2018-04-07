/*
 * Copyright 2017, alex at staticlibs.net
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* 
 * File:   jni_config.hpp
 * Author: alex
 *
 * Created on January 10, 2017, 9:48 PM
 */

#ifndef WILTON_JNI_WILTON_JNI_CONFIG_HPP
#define WILTON_JNI_WILTON_JNI_CONFIG_HPP

// helper macros
#define WILTON_QUOTE(value) #value
#define WILTON_STR(value) WILTON_QUOTE(value)
#define WILTON_PASTER(x,y) Java_ ## x ## _ ## y
#define WILTON_EVALUATOR(x,y) WILTON_PASTER(x,y)
#define WILTON_JNI_FUNCTION(fun) WILTON_EVALUATOR(WILTON_JNI_CLASS_NAME, fun)

// JNI version
#ifndef WILTON_JNI_VERSION
#define WILTON_JNI_VERSION JNI_VERSION_1_6
#endif // WILTON_JNI_VERSION

// name of the Java class that will expose `wiltoncall` method
#ifndef WILTON_JNI_CLASS_NAME
#define WILTON_JNI_CLASS_NAME wilton_WiltonJni
#endif // WILTON_JNI_CLASS_NAME

// signature of the Java class that will expose `wiltoncall` method
#ifndef WILTON_JNI_CLASS_SIGNATURE
#define WILTON_JNI_CLASS_SIGNATURE wilton/WiltonJni
#endif // WILTON_JNI_CLASS_SIGNATURE
#define WILTON_JNI_CLASS_SIGNATURE_STR WILTON_STR(WILTON_JNI_CLASS_SIGNATURE)

// method that returns stacktrace for specified java throwable
#ifndef WILTON_JNI_CLASS_DESCRIBETHROWABLE_METHOD
#define WILTON_JNI_CLASS_DESCRIBETHROWABLE_METHOD describeThrowable
#endif // WILTON_JNI_CLASS_DESCRIBETHROWABLE_METHOD
#define WILTON_JNI_CLASS_DESCRIBETHROWABLE_METHOD_STR WILTON_STR(WILTON_JNI_CLASS_DESCRIBETHROWABLE_METHOD)
#define WILTON_JNI_CLASS_DESCRIBETHROWABLE_METHOD_SIGNATURE (Ljava/lang/Throwable;)Ljava/lang/String;
#define WILTON_JNI_CLASS_DESCRIBETHROWABLE_METHOD_SIGNATURE_STR WILTON_STR(WILTON_JNI_CLASS_DESCRIBETHROWABLE_METHOD_SIGNATURE)

// signature of the gateway interface
#ifndef WILTON_JNI_GATEWAY_INTERFACE_SIGNATURE
#define WILTON_JNI_GATEWAY_INTERFACE_SIGNATURE wilton/WiltonGateway
#endif // WILTON_JNI_GATEWAY_INTERFACE_SIGNATURE
#define WILTON_JNI_GATEWAY_INTERFACE_SIGNATURE_STR WILTON_STR(WILTON_JNI_GATEWAY_INTERFACE_SIGNATURE)

// method that runs specified script
#ifndef WILTON_JNI_GATEWAY_RUNSCRIPT_METHOD
#define WILTON_JNI_GATEWAY_RUNSCRIPT_METHOD runScript
#endif // WILTON_JNI_GATEWAY_RUNSCRIPT_METHOD
#define WILTON_JNI_GATEWAY_RUNSCRIPT_METHOD_STR WILTON_STR(WILTON_JNI_GATEWAY_RUNSCRIPT_METHOD)
#define WILTON_JNI_GATEWAY_RUNSCRIPT_METHOD_SIGNATURE (Ljava/lang/String;)Ljava/lang/String;
#define WILTON_JNI_GATEWAY_RUNSCRIPT_METHOD_SIGNATURE_STR WILTON_STR(WILTON_JNI_GATEWAY_RUNSCRIPT_METHOD_SIGNATURE)

// exception class signature
#ifndef WILTON_JNI_EXCEPTION_CLASS_SIGNATURE
#define WILTON_JNI_EXCEPTION_CLASS_SIGNATURE wilton/WiltonException
#endif // WILTON_JNI_EXCEPTION_CLASS_SIGNATURE
#define WILTON_JNI_EXCEPTION_CLASS_SIGNATURE_STR WILTON_STR(WILTON_JNI_EXCEPTION_CLASS_SIGNATURE)

// directory to dump startup errors into
#ifndef WILTON_STARTUP_ERR_DIR
#define WILTON_STARTUP_ERR_DIR .
#endif // WILTON_STARTUP_ERR_DIR
#define WILTON_STARTUP_ERR_DIR_STR WILTON_STR(WILTON_STARTUP_ERR_DIR)


#endif /* WILTON_JNI_WILTON_JNI_CONFIG_HPP */
