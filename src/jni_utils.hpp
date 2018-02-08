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
 * File:   jni_utils.hpp
 * Author: alex
 *
 * Created on January 12, 2017, 3:11 PM
 */

#ifndef WILTON_JNI_JNI_UTILS_HPP
#define WILTON_JNI_JNI_UTILS_HPP

#include <string>

#include "jni.h"

#include "staticlib/config.hpp"

#include "wilton/support/exception.hpp"

namespace wilton {
namespace rhino {

inline std::string jstring_to_str(JNIEnv* env, jstring jstr) {
    const char* cstr = env->GetStringUTFChars(jstr, 0);
    size_t cstr_len = static_cast<size_t> (env->GetStringUTFLength(jstr));
    std::string res{cstr, cstr_len};
    env->ReleaseStringUTFChars(jstr, cstr);
    return res;
}

inline jclass find_java_class(JNIEnv* env, const std::string& name) {
    jclass local = env->FindClass(name.c_str());
    if (nullptr == local) {
        throw support::exception(TRACEMSG("Cannot load class, name: [" + name + "]"));
    }
    jclass global = static_cast<jclass>(env->NewGlobalRef(local));
    if (nullptr == global) {
        throw support::exception(TRACEMSG("Cannot create global ref for class, name: [" + name + "]"));
    }
    env->DeleteLocalRef(local);
    return global;
}

inline jmethodID find_java_method(JNIEnv* env, jclass clazz, const std::string& name, const std::string& signature) {
    jmethodID res = env->GetMethodID(clazz, name.c_str(), signature.c_str());
    if (nullptr == res) {
        throw support::exception(TRACEMSG("Cannot find method, name: [" + name + "]," +
                " signature: [" + signature + "]"));
    }
    return res;
}

inline jmethodID find_java_method_static(JNIEnv* env, jclass clazz, const std::string& name, const std::string& signature) {
    jmethodID res = env->GetStaticMethodID(clazz, name.c_str(), signature.c_str());
    if (nullptr == res) {
        throw support::exception(TRACEMSG("Cannot find static method, name: [" + name + "]," +
                " signature: [" + signature + "]"));
    }
    return res;
}

} // namespace
}

#endif /* WILTON_JNI_JNI_UTILS_HPP */

