
/*

  Twist Blink

 */
 
 #define MAX_BEAT 2500
#define MIN_BEAT 1000
#define MAX_LEVEL 30
#define DELAY_TIME 150
enum gameState {SETUP, DRUM, TWIST, PULL, DEAD};  //3 Game states for 3 Blinks + Setup + Dead
byte gameState = SETUP;
byte neightbors = 0;
byte receptionface = 6;
byte levelbeats = 0;
byte levelfases = 0;
byte actualbeat = 0;
byte actualfase = 0;
byte beat = 0;
byte level = 1;
byte levelup = 0;
int timing = 0;
byte start = 0;
byte lives = 3;
byte difficulty = 0;
byte sendData = 0;

byte drum = 0;
byte twist = 0;
byte pull = 0;

byte side = 0;

Timer beatTimer;
Timer startTimer;
Timer deadTimer;
Timer faseTimer;
Timer delayTimer;

void setup() {   randomize();  } // put your setup code here, to run once:

void loop() {  // put your main code here, to run repeatedly:
  switch (gameState) {
    case SETUP:              resetLoop();                                                     break;
    case DRUM:               drumLoop();   if (start != 2){setValueSentOnAllFaces(sendData);} break;
    case TWIST:  case PULL:  sideLoop();                   setValueSentOnAllFaces(sendData);  break;
    case DEAD:               deadLoop();                   setValueSentOnAllFaces(sendData);  break;
  }
  visualLoop();
}

void resetLoop(){
  neightbors = 0;
  lives = 3;
  level = 0;
  difficulty = 0;
  twist = 0; pull = 0; drum = 0;
  FOREACH_FACE(f) {
    if ( !isValueReceivedOnFaceExpired( f ) ) {
        neightbors++;
        byte neighborSide = getSide(getLastValueReceivedOnFace(f)); 
        if (neighborSide == 1){gameState = TWIST;  }
        if (neighborSide == 2){gameState = PULL;  }
        if (neightbors == 2){
          setValueSentOnFace(1, 0);setValueSentOnFace(1, 1);setValueSentOnFace(1, 2);
          setValueSentOnFace(2, 3);setValueSentOnFace(2, 4);setValueSentOnFace(2, 5);
          if (buttonSingleClicked()) { gameState = DRUM;  start = 2; }
        }
    }
  }  
}
void deadLoop(){
  sendData = (1 << 3);
  if (deadTimer.isExpired()){ gameState = SETUP; }
}

void drumLoop() {
  timing = MAX_BEAT - level*((MAX_BEAT-MIN_BEAT)/MAX_LEVEL);  
  if (start == 2){
    if (isAlone()){gameState = SETUP;}
    setValueSentOnFace(1, 0);setValueSentOnFace(1, 1);setValueSentOnFace(1, 2);
    setValueSentOnFace(2, 3);setValueSentOnFace(2, 4);setValueSentOnFace(2, 5);
    if (buttonSingleClicked()){difficulty = (difficulty+1)%3;}
    if (buttonDoubleClicked()){start = 0;
       if (difficulty == 0)  {level = 1;   lives = 3;}
       if (difficulty == 1)  {level = 11;  lives = 2;}
       if (difficulty == 2)  {level = 21;  lives = 1;}
       sendData = ((difficulty+1) << 4);
       delayTimer.set(DELAY_TIME);
    }
    if (buttonPressed()) {};
    timing = MAX_BEAT - level*((MAX_BEAT-MIN_BEAT)/MAX_LEVEL);
    startTimer.set(timing);
  }
  if (start == 0){ actualbeat = 0; actualfase = 1;
    if (delayTimer.isExpired()){sendData = 0;}
    if (startTimer.isExpired()){start = 1; beatTimer.set(DELAY_TIME);
      if (level < 11){ // BEGGINER
        levelbeats = random(2) + 4; 
        if (level < 6) {levelfases = 1;} 
        if (level > 5) {levelfases = 2;}
      }
      if (level > 10 && level < 21){ //ADVANCED
        levelbeats = random(3) + 5; 
        if (level < 16) {levelfases = 3;} 
        if (level > 15) {levelfases = 4;}
      }
      if (level > 20 && level < 31){ //EXPERT
        levelbeats = random(4) + 6; 
        levelfases = 5;
      }
      if (level > MAX_LEVEL){ //INFINITE MODE
        levelbeats = random(5) + 7; 
        levelfases = 7;
      }
    }
  }
  if (start == 1){
    if (actualbeat < levelbeats){
      if (drum == 0){
        if (beatTimer.isExpired()){
          beat = random(2);
          beatTimer.set(timing);
          if (beat == 0){drum = 1; twist = 0; pull = 0; side = 0;}
          if (beat == 1){drum = 0; twist = 1; pull = 0; side = 1;}
          if (beat == 2){drum = 0; twist = 0; pull = 1; side = 1;}
          sendData = beat;
          delayTimer.set(DELAY_TIME);
        }
        if (buttonSingleClicked()){}
      }
      if (drum == 1){
        if (beatTimer.getRemaining()>DELAY_TIME && (buttonSingleClicked())) {drum = 0; twist = 0; pull = 0; beatTimer.set(DELAY_TIME*2); actualbeat++;}
        if (beatTimer.getRemaining()<DELAY_TIME)                            {drum = 0; twist = 0; pull = 0; beatTimer.set(DELAY_TIME*2); lives--; }
      }
      if (side == 1){ if (delayTimer.isExpired()){sendData = 0;}
        FOREACH_FACE(f) {
          if ( !isValueReceivedOnFaceExpired( f ) ) {
            byte neighborBeat = getBeat(getLastValueReceivedOnFace(f));
            if (neighborBeat == 1)              {side = 0; drum = 0; twist = 0; pull = 0;        beatTimer.set(DELAY_TIME*2); actualbeat++; }
            if (neighborBeat == 2)              {side = 0; drum = 0; twist = 0; pull = 0;        beatTimer.set(DELAY_TIME*2); actualbeat++; }
          }
        }
        if (beatTimer.getRemaining()<DELAY_TIME){side = 0; drum = 0; twist = 0; pull = 0;        beatTimer.set(DELAY_TIME*2); lives--;}
      }
    }
    if (actualbeat == levelbeats){
      if (actualfase == levelfases){level++; start = 0; timing = MAX_BEAT - level*((MAX_BEAT-MIN_BEAT)/MAX_LEVEL); startTimer.set(timing); delayTimer.set(DELAY_TIME*2); sendData = (1 << 2);}
      if (actualfase != levelfases){actualbeat = 0; actualfase++; faseTimer.set(timing-DELAY_TIME); beatTimer.set(timing);}
    }
    if (lives == 0){gameState = DEAD; deadTimer.set(MAX_BEAT*2);}
  }
}

void sideLoop() {
  timing = MAX_BEAT - level*((MAX_BEAT-MIN_BEAT)/MAX_LEVEL); 
  if (level == 0){
    if (isAlone()){gameState = SETUP;}
    FOREACH_FACE(f) {
      if ( !isValueReceivedOnFaceExpired( f ) ) {
        byte neighborDiff = getDiff(getLastValueReceivedOnFace(f));
        switch (neighborDiff) {  case 1: level=1; break;        case 2: level=11; break;        case 3: level=21; break; }
      }
    }
  }
  if (level > 0){
    if (pull == 0 && twist == 0){
      if (beatTimer.isExpired()){levelup = 0; sendData = 0;
        FOREACH_FACE(f) {
          if ( !isValueReceivedOnFaceExpired( f ) ) {
            byte neighborBeat = getBeat(getLastValueReceivedOnFace(f));
            if (neighborBeat == 2 && gameState == PULL) {pull = 1;  beatTimer.set(timing-DELAY_TIME);}
            if (neighborBeat == 1 && gameState == TWIST){twist = 1; beatTimer.set(timing-DELAY_TIME);}
            byte neighborLevelup = getLevelup(getLastValueReceivedOnFace(f));
            if (neighborLevelup == 1){levelup = 1; level++; timing = MAX_BEAT - level*((MAX_BEAT-MIN_BEAT)/MAX_LEVEL); beatTimer.set(timing-DELAY_TIME);}
            byte neighborDead = getDead(getLastValueReceivedOnFace(f));
            if (neighborDead == 1){gameState = DEAD; deadTimer.set(MAX_BEAT*2);}
            receptionface = f;
          }
        }
      }
    }
    if (pull > 0 || twist > 0) {
      if (!beatTimer.isExpired()){
        FOREACH_FACE(f) {
          if ( !isValueReceivedOnFaceExpired( f ) ) {
            if (receptionface == f && gameState == PULL  && pull == 2 ){pull = 0;  sendData = 2; beatTimer.set(DELAY_TIME);}
            if (receptionface != f && gameState == TWIST && twist == 2){twist = 0; sendData = 1; beatTimer.set(DELAY_TIME);}
          }
        }
        if (isAlone() && gameState == PULL) { pull  = 2;  }
        if (isAlone() && gameState == TWIST){ twist = 2; }
      }
      if (beatTimer.isExpired()){
        pull = 0; twist = 0; 
      }
    }
  }
}

void visualLoop(){
  setColor(OFF);
  switch (gameState) {
    case SETUP:
      setColor(RED); setColorOnFace(GREEN,4); setColorOnFace(YELLOW,1);
      break;
    case DRUM:
      if (!faseTimer.isExpired()){
        setColor(dim(WHITE, ((MAX_BRIGHTNESS * faseTimer.getRemaining()) / timing)));
      }
      if (!startTimer.isExpired()){
        setColorOnFace(WHITE,random(5));
      }
      if (start == 2){
        if (difficulty == 0) setColor(GREEN);
        if (difficulty == 1) setColor(YELLOW);
        if (difficulty == 2) setColor(RED);
      }
      if (drum == 1){setColor(dim(RED, ((MAX_BRIGHTNESS * beatTimer.getRemaining()) / timing)));}
      if (twist == 1){
        setColorOnFace(dim(YELLOW, ((MAX_BRIGHTNESS * beatTimer.getRemaining()) / timing)), 1); 
//        setColorOnFace(dim(YELLOW, ((MAX_BRIGHTNESS * beatTimer.getRemaining()) / timing)), 0); 
//        setColorOnFace(dim(YELLOW, ((MAX_BRIGHTNESS * beatTimer.getRemaining()) / timing)), 2); 
      }
      if (pull == 1){
        setColorOnFace(dim(GREEN, ((MAX_BRIGHTNESS * beatTimer.getRemaining()) / timing)), 4); 
//        setColorOnFace(dim(GREEN, ((MAX_BRIGHTNESS * beatTimer.getRemaining()) / timing)), 3); 
//        setColorOnFace(dim(GREEN, ((MAX_BRIGHTNESS * beatTimer.getRemaining()) / timing)), 5); 
      }
      break;
    case TWIST:
      if (level == 0){setColor(dim(YELLOW,100));}
      if (levelup == 1){setColorOnFace(WHITE,random(5));}
      if (twist != 0 ){
//        setColorOnFace(dim(YELLOW, ((MAX_BRIGHTNESS * beatTimer.getRemaining()) / timing)/2), (receptionface+1)%6);
        setColorOnFace(dim(YELLOW, ((MAX_BRIGHTNESS * beatTimer.getRemaining()) / timing)), (receptionface+2)%6);
        setColorOnFace(dim(YELLOW, ((MAX_BRIGHTNESS * beatTimer.getRemaining()) / timing)), (receptionface+3)%6);
        setColorOnFace(dim(YELLOW, ((MAX_BRIGHTNESS * beatTimer.getRemaining()) / timing)), (receptionface+4)%6);
//        setColorOnFace(dim(YELLOW, ((MAX_BRIGHTNESS * beatTimer.getRemaining()) / timing)/2), (receptionface+5)%6);
      }
      break;
    case PULL:
      if (level == 0){setColor(dim(GREEN,100));}
      if (levelup == 1){setColorOnFace(WHITE,random(5));}
      if (pull == 1){
        setColorOnFace(dim(GREEN, ((MAX_BRIGHTNESS * beatTimer.getRemaining()) / timing)), (receptionface+2)%6);
        setColorOnFace(dim(GREEN, ((MAX_BRIGHTNESS * beatTimer.getRemaining()) / timing)), (receptionface+3)%6);
        setColorOnFace(dim(GREEN, ((MAX_BRIGHTNESS * beatTimer.getRemaining()) / timing)), (receptionface+4)%6);
      }
      if (pull == 2){
        setColorOnFace(dim(GREEN, ((MAX_BRIGHTNESS * beatTimer.getRemaining()) / timing)), (receptionface)%6);
        setColorOnFace(dim(GREEN, ((MAX_BRIGHTNESS * beatTimer.getRemaining()) / timing)), (receptionface+1)%6);
        setColorOnFace(dim(GREEN, ((MAX_BRIGHTNESS * beatTimer.getRemaining()) / timing)), (receptionface+5)%6);
      }
      break;
    case DEAD:
        setColorOnFace(RED,random(5));
    break;
  }
}

//COMUNICATIONS
byte getBeat(byte data) {return (data & 3);}//BYTE EF
byte getLevelup(byte data) {return (data >> 2 & 1);}//BYTE D
byte getDead(byte data) {return (data >> 3 & 1);}//BYTE C

byte getSide(byte data) {return (data & 3);}//BYTE EF
byte getDiff(byte data) {return (data >> 4 & 3);}//BYTE AB
