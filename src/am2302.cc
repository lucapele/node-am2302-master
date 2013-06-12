#include <wiringPi.h>
#include <stdint.h>
#include <v8.h>
#include <node.h>
#define MAX_TIMES 85

using namespace v8;

bool readData(int pin, float* result) {
    int data[5] = { 0, 0, 0, 0, 0 };
    uint8_t lststate = HIGH;  
    uint8_t counter = 0;  
    uint8_t j = 0, i;  

    pinMode(pin, OUTPUT);  
    digitalWrite(pin, LOW);  
    delay(18);
    digitalWrite(pin, HIGH);  
    delayMicroseconds(40);  
    pinMode(pin, INPUT);  

    for(i=0; i<MAX_TIMES; i++){
        counter=0;
        while(digitalRead(pin) == lststate){
            counter++;
            delayMicroseconds(1);
            if(counter==255){
                break;
            }
        }
        lststate=digitalRead(pin);
        if(counter==255){
            break;
        }
        if((i>=4)&&(i%2==0)){
            data[j/8]<<=1;
            if(counter>16){
                data[j/8]|=1;
            }
            j++;
        }
    }

    if((j >= 40) &&
            (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) ){

        result[0] = (data[0] * 256 + data[1]) / 10.0;
        result[1] = ((data[2] & 0x7F) * 256 + data[3]) / 10.0;
        if (data[2] & 0x80){
            result[1] *= -1;
        }
        return true;
    }
    return false;
}

Handle<Value> readMethod(const Arguments& args){
    HandleScope scope;
    int pin = 0, tryTimes = 3;
    float result[2];
    bool flag = false;
    Local<Object> data = Object::New();

    if(args.Length() == 0 || !args[0]->IsNumber()){
        ThrowException(Exception::TypeError(String::New("Wrong arguments")));
        return scope.Close(Boolean::New(false));
    }

    if(wiringPiSetup() == -1){
        ThrowException(Exception::TypeError(String::New("Initialize GPIO fail")));
        return scope.Close(Boolean::New(false));
    }
    pin = args[0]->NumberValue();
    while(!(flag = readData(pin, result)) && tryTimes--){
        delay(500);
    }
    if(flag){
        data->Set(String::New("h"), Number::New(result[0]));
        data->Set(String::New("t"), Number::New(result[1]));
        return scope.Close(data);
    }

    ThrowException(Exception::TypeError(String::New("Read data error")));
    return scope.Close(Undefined());
}

void initMethod(Handle<Object> exports){
    exports->Set(String::NewSymbol("read"),
            FunctionTemplate::New(readMethod)->GetFunction());
}

NODE_MODULE(am2302, initMethod)
