/* 
 * File:   wiltoncall_jni.cpp
 * Author: alex
 *
 * Created on January 10, 2017, 6:34 PM
 */

#include <atomic>
#include <memory>
#include <string>

#include "jni.h"

#include "staticlib/config.hpp"
#include "staticlib/io.hpp"
#include "staticlib/tinydir.hpp"
#include "staticlib/utils.hpp"

#include "wilton/wilton.h"
#include "wilton/wiltoncall.h"

#include "wilton/support/buffer.hpp"
#include "wilton/support/exception.hpp"
#include "wilton/support/registrar.hpp"

#include "jni_config.hpp"
#include "jni_utils.hpp"

namespace wilton{
namespace rhino {

namespace { // anonymous

std::atomic<bool>& static_jvm_active() {
    static std::atomic<bool> flag{false};
    return flag;
}

// forward declaration
JNIEnv* get_jni_env();

class global_ref_deleter {
public:
    void operator()(jobject ref) {
        if (static_jvm_active().load()) {
            get_jni_env()->DeleteGlobalRef(ref);
        }
    }
};

class jni_ctx {
public:
    JavaVM* vm;
    std::unique_ptr<_jclass, global_ref_deleter> wiltonJniClass;
    jmethodID describeThrowableMethod;
    std::unique_ptr<_jclass, global_ref_deleter> wiltonGatewayInterface;
    jmethodID runScriptMethod;
    std::unique_ptr<_jclass, global_ref_deleter> wiltonExceptionClass;
    std::unique_ptr<_jobject, global_ref_deleter> wiltonGatewayObject;

    jni_ctx(const jni_ctx&) = delete;

    jni_ctx& operator=(const jni_ctx&) = delete;

    jni_ctx(jni_ctx&&) = delete;

    jni_ctx& operator=(jni_ctx&& other) {
        this->vm = other.vm;
        other.vm = nullptr;
        this->wiltonJniClass = std::move(other.wiltonJniClass);
        this->describeThrowableMethod = other.describeThrowableMethod;
        other.describeThrowableMethod = nullptr;
        this->wiltonGatewayInterface = std::move(other.wiltonGatewayInterface);
        this->runScriptMethod = other.runScriptMethod;
        other.runScriptMethod = nullptr;
        this->wiltonExceptionClass = std::move(other.wiltonExceptionClass);
        this->wiltonGatewayObject = std::move(other.wiltonGatewayObject);
        return *this;
    }

    jni_ctx() { }

    jni_ctx(JavaVM* vm) :
    vm(vm) {
        // env
        JNIEnv* env;
        auto err = vm->GetEnv(reinterpret_cast<void**> (std::addressof(env)), WILTON_JNI_VERSION);
        if (JNI_OK != err) {
            throw support::exception(TRACEMSG("Cannot obtain JNI environment"));
        }
        // jni class
        this->wiltonJniClass = std::unique_ptr<_jclass, global_ref_deleter>(
                find_java_class(env, WILTON_JNI_CLASS_SIGNATURE_STR), global_ref_deleter());
        // describe
        this->describeThrowableMethod = find_java_method_static(env, this->wiltonJniClass.get(),
                WILTON_JNI_CLASS_DESCRIBETHROWABLE_METHOD_STR, WILTON_JNI_CLASS_DESCRIBETHROWABLE_METHOD_SIGNATURE_STR);
        // gateway
        this->wiltonGatewayInterface = std::unique_ptr<_jclass, global_ref_deleter>(
                find_java_class(env, WILTON_JNI_GATEWAY_INTERFACE_SIGNATURE_STR), global_ref_deleter());
        // runscript
        this->runScriptMethod = find_java_method(env, this->wiltonGatewayInterface.get(),
                WILTON_JNI_GATEWAY_RUNSCRIPT_METHOD_STR, WILTON_JNI_GATEWAY_RUNSCRIPT_METHOD_SIGNATURE_STR);
        // exception
        this->wiltonExceptionClass = std::unique_ptr<_jclass, global_ref_deleter>(
                find_java_class(env, WILTON_JNI_EXCEPTION_CLASS_SIGNATURE_STR), global_ref_deleter());
    }
    
    void set_gateway_object(JNIEnv* env, jobject gateway) {
        auto ptr = std::unique_ptr<_jobject, global_ref_deleter>(
                static_cast<jobject> (env->NewGlobalRef(gateway)), global_ref_deleter());
        if (nullptr == ptr.get()) {
            throw support::exception(TRACEMSG("Cannot create global ref for specified gateway object"));
        }
        this->wiltonGatewayObject = std::move(ptr);
    }
};

jni_ctx& static_jni_ctx() {
    // will be destructed in JNI_OnUnload
    static jni_ctx* ctx = new jni_ctx();
    return *ctx;
}

JNIEnv* get_jni_env() {
    JavaVM* vm = static_jni_ctx().vm;
    JNIEnv* env;
    auto getenv_err = vm->GetEnv(reinterpret_cast<void**> (std::addressof(env)), WILTON_JNI_VERSION);
    switch (getenv_err) {
    case JNI_OK:
        return env;
    case JNI_EDETACHED:
        if (JNI_OK == vm->AttachCurrentThread(reinterpret_cast<void**>(std::addressof(env)), nullptr)) {
            return env;
        }
        // fall-through to report error to client
    default:
        throw support::exception(TRACEMSG("System error: cannot obtain JNI environment"));
    }
}

std::string describe_java_exception(JNIEnv* env, jthrowable exc) {
    jobject umsg = env->CallStaticObjectMethod(static_jni_ctx().wiltonJniClass.get(),
            static_jni_ctx().describeThrowableMethod, exc);
    if (!env->ExceptionCheck()) {
        std::string res = jstring_to_str(env, static_cast<jstring>(umsg));
        env->DeleteLocalRef(umsg);
        return res;
    } else {
        env->ExceptionClear();
        return "EXC_DESCRIBE_ERROR";
    }
}

void dump_error(const std::string& directory, const std::string& msg) {
    try {
        // random postfix
        std::string id = sl::utils::random_string_generator().generate(12);
        auto errfile = directory + "wilton_ERROR_" + id + ".txt";
        auto fd = sl::tinydir::file_sink(errfile);
        sl::io::write_all(fd, msg);
    } catch (...) {
        // give up
    }
}

} // namespace

support::buffer runscript(sl::io::span<const char> data) {
    jni_ctx& ctx = static_jni_ctx();
    JNIEnv* env = get_jni_env();
    auto json_in = std::string(data.data(), data.size());
    jstring json_ustr = env->NewStringUTF(json_in.c_str());
    jobject res = env->CallObjectMethod(ctx.wiltonGatewayObject.get(),
            ctx.runScriptMethod, json_ustr);
    env->DeleteLocalRef(json_ustr);
    jthrowable exc = env->ExceptionOccurred();
    if (nullptr == exc) {
        if (nullptr !=  res) {
            std::string res_str = wilton::rhino::jstring_to_str(env, static_cast<jstring> (res));
            return support::make_string_buffer(res_str);
        } else {
            return support::make_empty_buffer();
        }
    } else {
        env->ExceptionClear();
        std::string trace = describe_java_exception(env, exc);
        throw support::exception(TRACEMSG(trace));
    }
}

} // namespace
}

extern "C" {

jint JNICALL JNI_OnLoad(JavaVM* vm, void*) {
    try {
        // move-assign static ctx
        wilton::rhino::static_jni_ctx() = wilton::rhino::jni_ctx(vm);
        // set init flag
        wilton::rhino::static_jvm_active().store(true);
        return WILTON_JNI_VERSION;
    } catch (const std::exception& e) {
        wilton::rhino::dump_error(WILTON_STARTUP_ERR_DIR_STR, TRACEMSG(e.what() + "\nInitialization error"));
        return -1;
    }
}

// generally won't be called on most JVMs
// as ctx holds a bunch of global refs
void JNICALL JNI_OnUnload(JavaVM*, void*) {
    delete std::addressof(wilton::rhino::static_jni_ctx());
    // flip init flag
    wilton::rhino::static_jvm_active().store(false);
}

void JNICALL WILTON_JNI_FUNCTION(wiltoninit)
(JNIEnv* env, jclass, jobject gateway, jstring config) {
    // check called once
    bool the_false = false;
    static std::atomic<bool> initilized{false};
    if (!initilized.compare_exchange_strong(the_false, true)) {
        env->ThrowNew(wilton::rhino::static_jni_ctx().wiltonExceptionClass.get(),
                TRACEMSG("'wiltoninit' was already called").c_str());
        return;
    }
    
    std::string conf = "";
    try {
        // set gateway
        wilton::rhino::static_jni_ctx().set_gateway_object(env, gateway);
        // wiltoncalls
        conf = wilton::rhino::jstring_to_str(env, config);
        auto err_init = wiltoncall_init(conf.c_str(), static_cast<int>(conf.length()));
        if (nullptr != err_init) {
            wilton::support::throw_wilton_error(err_init, TRACEMSG(err_init));
        }
        wilton::support::register_wiltoncall("runscript_rhino", wilton::rhino::runscript);
    } catch (const std::exception& e) {
        env->ThrowNew(wilton::rhino::static_jni_ctx().wiltonExceptionClass.get(),
                TRACEMSG(e.what() + "\nWilton initialization error," +
                " conf: [" + conf + "]").c_str());
    }
}

jstring JNICALL WILTON_JNI_FUNCTION(wiltoncall)
(JNIEnv* env, jclass, jstring name, jstring data) {
    if (nullptr == name) {
        env->ThrowNew(wilton::rhino::static_jni_ctx().wiltonExceptionClass.get(),
                TRACEMSG("Null 'name' parameter specified").c_str());
        return nullptr;
    }
    if (nullptr == data) {
        env->ThrowNew(wilton::rhino::static_jni_ctx().wiltonExceptionClass.get(),
                TRACEMSG("Null 'data' parameter specified").c_str());
        return nullptr;
    }
    std::string name_string = wilton::rhino::jstring_to_str(env, name);
    std::string data_string = wilton::rhino::jstring_to_str(env, data);
    char* out = nullptr;
    int out_len = 0;
    auto err = wiltoncall(name_string.c_str(), static_cast<int>(name_string.length()), 
            data_string.c_str(), static_cast<int>(data_string.length()),
            std::addressof(out), std::addressof(out_len));
    if (nullptr == err) {
        if (nullptr != out) {
            // todo: prepare NUL-teminated MUTF-8 before passing
            jstring res = env->NewStringUTF(out);
            wilton_free(out);
            return res;
        } else {
            return nullptr;
        }
    } else {
        env->ThrowNew(wilton::rhino::static_jni_ctx().wiltonExceptionClass.get(), TRACEMSG(err + 
                "\n'wiltoncall' error for name: [" + name_string + "]").c_str());
        wilton_free(err);
        return nullptr;
    }
}

} // C
