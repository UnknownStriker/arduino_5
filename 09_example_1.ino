// Arduino pin assignment
#define PIN_LED  9
#define PIN_TRIG 12
#define PIN_ECHO 13
#define _EMA_ALPHA 0.5
// configurable parameters
#define SND_VEL 346.0
#define INTERVAL 25
#define PULSE_DURATION 10
#define _DIST_MIN 100
#define _DIST_MAX 300

#define TIMEOUT ((INTERVAL / 2) * 1000.0)
#define SCALE (0.001 * 0.5 * SND_VEL)

// 중위수 필터의 샘플 개수: 홀수 권장
#define N 30

// global variables
unsigned long last_sampling_time = 0;

float samples[N];
int next_index = 0;
int sample_count = 0;
float dist_ema; 

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  digitalWrite(PIN_TRIG, LOW);
  digitalWrite(PIN_LED, HIGH);  // LED OFF (기존 회로 기준)

  Serial.begin(57600);
}

void loop() {
  unsigned long now = millis();


  if (now - last_sampling_time < INTERVAL)
    return;

  last_sampling_time = now;


  float dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);

  float dist_filtered = median_filter(dist_raw);

  dist_ema = _EMA_ALPHA * dist_filtered + (1 - _EMA_ALPHA) * dist_ema;

  Serial.print("Min:");
  Serial.print(_DIST_MIN);

  Serial.print(",raw:");
  Serial.print(dist_raw);

  Serial.print(",median:");
  Serial.print(dist_filtered);

  Serial.print(",ema:");  
  Serial.print(dist_ema);
  Serial.print(",Max:");
  Serial.println(_DIST_MAX);
  Serial.println("");
  
  if ((dist_raw < _DIST_MIN) || (dist_raw > _DIST_MAX))
    digitalWrite(PIN_LED, HIGH);  // LED OFF
  else
    digitalWrite(PIN_LED, LOW);   // LED ON
}

float median_filter(float new_sample) {
  samples[next_index] = new_sample;
  next_index = (next_index + 1) % N;

  if (sample_count < N)
    sample_count++;

  float sorted[N];

  for (int i = 0; i < sample_count; i++)
    sorted[i] = samples[i];

  for (int i = 1; i < sample_count; i++) {
    float value = sorted[i];
    int j = i - 1;

    while (j >= 0 && sorted[j] > value) {
      sorted[j + 1] = sorted[j];
      j--;
    }

    sorted[j + 1] = value;
  }

  if (sample_count % 2 == 1)
    return sorted[sample_count / 2];

  return (sorted[sample_count / 2 - 1]
        + sorted[sample_count / 2]) / 2.0;
}

float USS_measure(int TRIG, int ECHO) {
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);

  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE;
}
