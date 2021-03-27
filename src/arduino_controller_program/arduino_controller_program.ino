void setup(){
  Serial.begin(9600);
  pinMode(7, OUTPUT);//设置13号端口作为输出端口
  digitalWrite(7,HIGH);//让灯开始时无
}
char var;
void loop(){
  digitalWrite(7,HIGH);//让灯开始时无
  while(Serial.available()>0)//当有信号的时候
  {
    var=Serial.read();
    if(var=='0'){//传过来的是0，即不工作
      digitalWrite(7,HIGH);
//      delay(5000); //5s延迟，多放点
    }
    if(var=='1'){//传过来的是1，即工作
      digitalWrite(7,LOW);
      delay(5000);
    }
  }
}
