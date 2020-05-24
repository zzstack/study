#include <string>
#include <node.h>
#include <v8.h>
//#include <nan.h>

using namespace v8;

void helloWorld(const v8::FunctionCallbackInfo<Value>& args) 
{
    Isolate* isolate = Isolate::GetCurrent();
    //Isolate* isolate = args.GetIsolate();
    //BrowserWindow.getNativeWindowHandle();
    HandleScope scope(isolate);
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, " nodejs native hello world "));
}

void accumulate(const v8::FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    /* 提取通过参数传递的回调函数 */
    Local<Function> callback = Local<Function>::Cast(args[args.Length() - 1]);

    /* 遍历参数进行求和 */
    double sum = 0.0;
    for (int i = 0; i < args.Length() - 1; ++i)
    {
        sum += args[i]->NumberValue();
    }

    /* 将求和结果转成一个js Number, 通过回调函数进行返回 */
    Local<Number> num = Number::New(isolate, sum);
    Local<Value> argv[1] = { num };
    callback->Call(Null(isolate), 1, argv);
}

std::string getArgType(const Local<v8::Value>& arg)
{
    if(arg->IsString()) return "String";
    if(arg->IsBooleanObject()) return "BooleanObject";
    if(arg->IsStringObject()) return "StringObject";
    if(arg->IsObject()) return "Object";
    return "Unknown";
}

// http://thlorenz.com/v8-dox/build/v8-3.14.5/html/dc/d0a/classv8_1_1_value.html
void showVideo(const v8::FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    HandleScope scope(isolate);
    if(1 != args.Length())
    {
        return;
    }
    //Local<Object> handler = args[0]->ToObject();

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, getArgType(args[0]).c_str()));
}

void Init(Handle<Object> exports) 
{
    Isolate* isolate = Isolate::GetCurrent();
    exports->Set(String::NewFromUtf8(isolate, "hello"),
    FunctionTemplate::New(isolate, helloWorld)->GetFunction());

    // NODE_SET_METHOD用于设置属性或方法，第二个参数为属性名，第三个参数为方法对应的属性值。如果需要给exports对象设置多个属性或方法，可以调用多次NODE_SET_METHOD。exports对象上设置的属性方法将会作为接口暴露给外部使用。
    NODE_SET_METHOD(exports, "accumulate", accumulate);
    NODE_SET_METHOD(exports, "showVideo", showVideo);
}

// NODE_MODULE设置模块初始函数为Init
NODE_MODULE(hello, Init)