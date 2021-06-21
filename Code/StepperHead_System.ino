byte rotation_direction = 0, steps_mode = 0, i = 0;
unsigned short rotation_time = 0, rotation_angle = 0;
unsigned short steps_count = 0, steps_delay = 0,steps_count_x = 0;
bool receiving = false, action = false;

byte half_steps[8] = {
  0b00001000,
  0b00001100,
  0b00000100,
  0b00000110,
  0b00000010,
  0b00000011,
  0b00000001,
  0b00001001,
};


void setup() {
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  
  Serial1.begin(9600);
  sendMessage("Głowica jest gotowa");
}

void loop() {
  receive();
  if (action) stepper();
}

void stepper()
{
  switch (steps_mode) {
    case 0: stop_motor();
      break;
    case 1: right(steps_delay, i);
      break;
    case 2: left(steps_delay, i);
      break;
    case 3: test(i);
      break;
    default: sendMessage("Tryb z poza zakresu");
      break;
  }

  if (steps_count > 1)
  {
    if (steps_count == steps_count_x) sendMessage("Głowica uruchomiona");
    steps_count--;
  }
  else
  {
    stop_motor();
  }

  i = i > 6 ? 0 : i + 1;
}

void left(unsigned short delay_time, byte i)
{
  digitalWrite(7, bitRead(half_steps[i], 0));
  digitalWrite(6, bitRead(half_steps[i], 1));
  digitalWrite(5, bitRead(half_steps[i], 2));
  digitalWrite(4, bitRead(half_steps[i], 3));
  delay(delay_time);
}

void right(unsigned short delay_time, byte i)
{
  digitalWrite(4, bitRead(half_steps[i], 0));
  digitalWrite(5, bitRead(half_steps[i], 1));
  digitalWrite(6, bitRead(half_steps[i], 2));
  digitalWrite(7, bitRead(half_steps[i], 3));
  delay(delay_time);
}

// Trzeba wyslac kod na 1ms 59904steps 3rotation
void test(byte i)
{
  if (steps_count < (steps_count_x / 4) && steps_count % 1500 == 0) steps_delay++; //koncowo bedzie 11ms delay
  right(steps_delay, i);
}

void stop_motor()
{
  steps_count = 0;
  steps_delay = 0;
  steps_count_x = 0;
  steps_mode = 0;
  action = false;
  digitalWrite(7, 0);
  digitalWrite(6, 0);
  digitalWrite(5, 0);
  digitalWrite(4, 0);
  sendMessage("Głowica zatrzymana");
}

void end_receive()
{
  receiving = false;
  action = true;

  steps_count = rotation_angle;
  steps_count_x = steps_count;
  steps_delay = rotation_time;
  steps_mode = rotation_direction;
}

void erase_data()
{
  rotation_angle = 0;
  rotation_time = 0;
  rotation_direction = 0;
}

void receive() {
  static byte command_byte = 0;
  byte rb;

  if (Serial1.available() > 0) {
    rb = Serial1.read();

    if (receiving)
    {
      switch (command_byte) {
        case 0:
          rotation_time = rb;
          break;
        case 1:
          rotation_time += rb << 8;
          break;
        case 2:
          rotation_angle = rb;
          break;
        case 3:
          rotation_angle += rb<< 8;
          break;
        case 4:
          rotation_direction = rb;
          end_receive();
          break;
        default:
          command_byte = 0;
          erase_data();
          receiving = false;
          break;
      }
      command_byte++;
    }
    else if (rb == 3)
    {
      erase_data();
      receiving = true;
      command_byte = 0;
    }
  }
}

void sendMessage(String text)
{
  Serial1.print("\n\n");
  Serial1.print(text);
}
