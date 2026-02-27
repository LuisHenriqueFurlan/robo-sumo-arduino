/*
============================================================
PROJETO: Robo Sumo
VERSAO: 5.0
AUTOR: Luis Henrique
DESCRICAO: Robo sumo autonomo com Motor Shield L293D,
sensor ultrassonico HC-SR04 e sensores de linha frontal
e traseiro.
============================================================
*/

#include <AFMotor.h>
#include <Ultrasonic.h>

#define TRIGGER A0
#define ECHO A1
#define SENSOR_FRENTE A4
#define SENSOR_TRAS A5

#define VEL_BUSCA 180
#define VEL_ATAQUE 255
#define VEL_MANOBRA 150
#define VEL_GIRO 200

#define TEMPO_GIRO 300
#define TEMPO_RECUO 600
#define TEMPO_AVANCO 600
#define DEBOUNCE_TEMPO 50

AF_DCMotor motor1(1);
AF_DCMotor motor2(2);
AF_DCMotor motor3(3);
AF_DCMotor motor4(4);
Ultrasonic ultrasonic(TRIGGER, ECHO);

unsigned long ultimoTempo = 0;
unsigned long ultimoDebounce = 0;
int ultimoSensorFrente = LOW;
int ultimoSensorTras = LOW;
bool sentidoBusca = true;

enum Estado { PROCURANDO, ATACANDO, RECUANDO, AVANCANDO, PARADO };
Estado estadoAtual = PROCURANDO;

void setup() {
  Serial.begin(9600);

  motor1.setSpeed(VEL_BUSCA);
  motor2.setSpeed(VEL_BUSCA);
  motor3.setSpeed(VEL_BUSCA);
  motor4.setSpeed(VEL_BUSCA);

  pinMode(SENSOR_FRENTE, INPUT);
  pinMode(SENSOR_TRAS, INPUT);

  delay(2000);
}

void loop() {
  float dist = ultrasonic.read();
  if (dist <= 0 || dist > 400) dist = 400;

  int sFrente = lerSensorComDebounce(SENSOR_FRENTE, ultimoSensorFrente);
  int sTras   = lerSensorComDebounce(SENSOR_TRAS, ultimoSensorTras);

  if (sFrente == HIGH) {
    mudarEstado(RECUANDO);
    executarEstado(dist, sFrente, sTras);
    return;
  }
  
  if (sTras == HIGH) {
    mudarEstado(AVANCANDO);
    executarEstado(dist, sFrente, sTras);
    return;
  }

  if (dist <= 20 && dist > 0) {
    mudarEstado(ATACANDO);
  } else if (estadoAtual != RECUANDO && estadoAtual != AVANCANDO) {
    mudarEstado(PROCURANDO);
  }

  executarEstado(dist, sFrente, sTras);
}

int lerSensorComDebounce(int pino, int &ultimoEstado) {
  int leitura = digitalRead(pino);
  
  if (leitura != ultimoEstado) {
    if (millis() - ultimoDebounce > DEBOUNCE_TEMPO) {
      ultimoEstado = leitura;
      ultimoDebounce = millis();
    }
  }
  
  return ultimoEstado;
}

void mudarEstado(Estado novo) {
  if (estadoAtual != novo) {
    estadoAtual = novo;
    ultimoTempo = millis();
  }
}

void executarEstado(float d, int sf, int st) {
  switch (estadoAtual) {
    case PROCURANDO: procura(); break;
    case ATACANDO: ataque(d, sf); break;
    case RECUANDO: recuar(st); break;
    case AVANCANDO: avancar(sf); break;
    case PARADO: parada(); break;
  }
}

void frente(int vel = VEL_BUSCA) {
  setVelocidade(vel);
  motor1.run(FORWARD);
  motor2.run(FORWARD);
  motor3.run(FORWARD);
  motor4.run(FORWARD);
}

void tras(int vel = VEL_MANOBRA) {
  setVelocidade(vel);
  motor1.run(BACKWARD);
  motor2.run(BACKWARD);
  motor3.run(BACKWARD);
  motor4.run(BACKWARD);
}

void esquerda(int vel = VEL_GIRO) {
  setVelocidade(vel);
  motor1.run(FORWARD);
  motor2.run(FORWARD);
  motor3.run(BACKWARD);
  motor4.run(BACKWARD);
}

void direita(int vel = VEL_GIRO) {
  setVelocidade(vel);
  motor1.run(BACKWARD);
  motor2.run(BACKWARD);
  motor3.run(FORWARD);
  motor4.run(FORWARD);
}

void parada() {
  motor1.run(RELEASE);
  motor2.run(RELEASE);
  motor3.run(RELEASE);
  motor4.run(RELEASE);
}

void setVelocidade(int vel) {
  motor1.setSpeed(vel);
  motor2.setSpeed(vel);
  motor3.setSpeed(vel);
  motor4.setSpeed(vel);
}

void procura() {
  if (millis() - ultimoTempo > 1500) {
    sentidoBusca = !sentidoBusca;
    ultimoTempo = millis();
  }
  if (sentidoBusca) direita(VEL_GIRO); else esquerda(VEL_GIRO);
}

void ataque(float dist, int sensorFrente) {
  if (dist <= 20 && sensorFrente == LOW) {
    frente(VEL_ATAQUE);
  } else {
    mudarEstado(PROCURANDO);
  }
}

void recuar(int sensorTras) {
  tras(VEL_MANOBRA);
  if (millis() - ultimoTempo > TEMPO_RECUO || sensorTras == LOW) {
    mudarEstado(PROCURANDO);
  }
}

void avancar(int sensorFrente) {
  frente(VEL_MANOBRA);
  if (millis() - ultimoTempo > TEMPO_AVANCO || sensorFrente == LOW) {
    mudarEstado(PROCURANDO);
  }
}
