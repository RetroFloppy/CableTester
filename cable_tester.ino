//
// ADTPro cable tester for Arduino, tuned in this case for the Teensy LC
//
// Assumes one of three "recipes" for cables
// If more valid types emerge, it may make more sense to have a function that takes a table and evaluates its truth.

// Debug stuff
boolean debug = false;
char message[250];
int val = 0;

const short DIN5_1 = 0, /* Female on board */
            DIN5_2 = 1,
            DIN5_3 = 2,
            DIN5_4 = 3,
            DIN5_5 = 4,
            DIN5_G = 9,
            MD8_1 = 0, /* Female on board */
            MD8_2 = 1,
            MD8_3 = 2,
            MD8_4 = 3,
            MD8_5 = 4,
            MD8_6 = 5,
            MD8_7 = 6,
            MD8_8 = 7,
            MD8_G = 9,
            DE9_1 = 10, /* Male on board */
            DE9_2 = 11,
            DE9_3 = 12,
            DE9_4 = 14,
            DE9_5 = 15,
            DE9_6 = 16,
            DE9_7 = 17,
            DE9_8 = 18,
            DE9_9 = 19,
            DE9_G = 20;
const short STATE_NOGOOD = 0,
            STATE_DIN5 = 1,
            STATE_MD8 = 2,
            STATE_NEWMD8 = 3;

const int led = LED_BUILTIN;
const int din5_pin_count = 16;
const int minidin8_pin_count = 19;
short din5_pin_assignments[] = { DIN5_1, DIN5_2, DIN5_3, DIN5_4, DIN5_5, DIN5_G, DE9_1, DE9_2, DE9_3, DE9_4, DE9_5, DE9_6, DE9_7, DE9_8, DE9_9, DE9_G };
char din5_pin_names[din5_pin_count][7] = { "DIN5_1", "DIN5_2", "DIN5_3", "DIN5_4", "DIN5_5", "DIN5_G", "DE9_1 ", "DE9_2 ", "DE9_3 ", "DE9_4 ", "DE9_5 ", "DE9_6", "DE9_7 ", "DE9_8 ", "DE9_9 ", "DE9_G " };
short minidin8_pin_assignments[] = { MD8_1, MD8_2, MD8_3, MD8_4, MD8_5, MD8_6, MD8_7, MD8_8, MD8_G, DE9_1, DE9_2, DE9_3, DE9_4, DE9_5, DE9_6, DE9_7, DE9_8, DE9_9, DE9_G };
char minidin8_pin_names[minidin8_pin_count][7] = { "MD8_1 ", "MD8_2 ", "MD8_3 ", "MD8_4 ", "MD8_5 ", "MD8_6 ", "MD8_7 ", "MD8_8 ", "MD8_G ", "DE9_1 ", "DE9_2 ", "DE9_3 ", "DE9_4 ", "DE9_5 ", "DE9_6", "DE9_7 ", "DE9_8 ", "DE9_9 ", "DE9_G " };
short state, new_state = STATE_NOGOOD;

// DIN5 table - maps the valid connections between the DIN5 and DE9 cable
boolean din5_truth_table[din5_pin_count][din5_pin_count] =
{ /*            5  5  5  5  5  5  9  9  9  9  9  9  9  9  9  9 */
  /*            /  /  /  /  /  /  /  /  /  /  /  /  /  /  /  / */
  /*            1  2  3  4  5  G  1  2  3  4  5  6  7  8  9  G */
  /* Row 0 */  {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // DIN5_1 out
  /* Row 1 */  {0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}, // DIN5_2 out
  /* Row 2 */  {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0}, // DIN5_3 out
  /* Row 3 */  {0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0}, // DIN5_4 out
  /* Row 4 */  {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // DIN5_5 out
  /* Row 5 */  {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, // DIN5_G out
  /* Row 6 */  {0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0}, // DE9_1 out
  /* Row 7 */  {0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}, // DE9_2 out
  /* Row 8 */  {0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0}, // DE9_3 out
  /* Row 9 */  {0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0}, // DE9_4 out
  /* Row 10 */ {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0}, // DE9_5 out
  /* Row 11 */ {0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0}, // DE9_6 out
  /* Row 12 */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0}, // DE9_7 out
  /* Row 13 */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0}, // DE9_8 out
  /* Row 14 */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0}, // DE9_9 out
  /* Row 15 */ {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}  // DE9_G out
};

// MiniDIN8 table - maps the valid connections between the DIN5 and MiniDIN8 cable
boolean minidin8_truth_table[minidin8_pin_count][minidin8_pin_count] =
{ /*            8  8  8  8  8  8  8  8  8  9  9  9  9  9  9  9  9  9  9  */
  /*            /  /  /  /  /  /  /  /  /  /  /  /  /  /  /  /  /  /  /  */
  /*            1  2  3  4  5  6  7  8  G  1  2  3  4  5  6  7  8  9  G  */
  /* Row 0 */  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0}, // MD8_1 out
  /* Row 1 */  {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0}, // MD8_2 out
  /* Row 2 */  {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}, // MD8_3 out
  /* Row 3 */  {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0}, // MD8_4 out
  /* Row 4 */  {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0}, // MD8_5 out
  /* Row 5 */  {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // MD8_6 out
  /* Row 6 */  {0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // MD8_7 out
  /* Row 7 */  {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0}, // MD8_8 out
  /* Row 8 */  {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, // MD8_G out
  /* Row 9 */  {0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // DE9_1 out
  /* Row 10 */ {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}, // DE9_2 out
  /* Row 11 */ {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0}, // DE9_3 out
  /* Row 12 */ {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0}, // DE9_4 out
  /* Row 13 */ {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0}, // DE9_5 out
  /* Row 14 */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0}, // DE9_6 out
  /* Row 15 */ {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0}, // DE9_7 out
  /* Row 16 */ {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0}, // DE9_8 out
  /* Row 17 */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0}, // DE9_9 out
  /* Row 18 */ {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}  // DE9_G out
};

// New MiniDIN8 table - maps the valid connections between the DIN5 and another MiniDIN8 cable
boolean new_minidin8_truth_table[minidin8_pin_count][minidin8_pin_count] =
{ /*            8  8  8  8  8  8  8  8  8  9  9  9  9  9  9  9  9  9  9  */
  /*            /  /  /  /  /  /  /  /  /  /  /  /  /  /  /  /  /  /  /  */
  /*            1  2  3  4  5  6  7  8  G  1  2  3  4  5  6  7  8  9  G  */
  /* Row 0 */  {1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0}, // MD8_1 out
  /* Row 1 */  {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0}, // MD8_2 out
  /* Row 2 */  {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}, // MD8_3 out
  /* Row 3 */  {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0}, // MD8_4 out
  /* Row 4 */  {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0}, // MD8_5 out
  /* Row 5 */  {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // MD8_6 out
  /* Row 6 */  {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // MD8_7 out
  /* Row 7 */  {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0}, // MD8_8 out
  /* Row 8 */  {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, // MD8_G out
  /* Row 9 */  {1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0}, // DE9_1 out
  /* Row 10 */ {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}, // DE9_2 out
  /* Row 11 */ {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0}, // DE9_3 out
  /* Row 12 */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0}, // DE9_4 out
  /* Row 13 */ {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0}, // DE9_5 out
  /* Row 14 */ {1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0}, // DE9_6 out
  /* Row 15 */ {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0}, // DE9_7 out
  /* Row 16 */ {1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0}, // DE9_8 out
  /* Row 17 */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0}, // DE9_9 out
  /* Row 18 */ {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}  // DE9_G out
};

void setup()
{
  for (int i = 0; i < minidin8_pin_count; i++)
  {
    // Initialize all pins to input; we will only toggle output on one pin at a time
    pinMode(minidin8_pin_assignments[i], INPUT_PULLUP);
  }
  Serial.begin(9600);
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
  state = STATE_NOGOOD;
  new_state = STATE_NOGOOD;
}

void loop()
{
  new_state = STATE_NOGOOD;
  if (isDIN5())
    new_state = STATE_DIN5;
  if (isMiniDIN8())
    new_state = STATE_MD8;
  if (isNewMiniDIN8())
    new_state = STATE_NEWMD8;
  state = new_state;
  blinkyBlink(state);
  delay(300);
  if (debug) delay(1000);
}

void blinkyBlink(short state)
{
  for (int i = 0; i < state; i++)
  {
    digitalWrite(led, true);
    delay(150);
    digitalWrite(led, false);
    if (i < state-1)
      delay(150);
  }
}

boolean isMiniDIN8()
{
  // Test MiniDIN8 cable

  // Innocent until proven guilty
  boolean cable_good = true;
  for (int i = 0; i < minidin8_pin_count; i++ )
  {
    if (debug)
    {
      sprintf(message, "Row %d, pin %s:", i, minidin8_pin_names[i]);
      Serial.println(message);
    }
    pinMode(minidin8_pin_assignments[i], OUTPUT);
    delay(1);  // Need to give the pin time to settle
    digitalWrite(minidin8_pin_assignments[i], LOW);  // We're using pullup, so invert logic
    // Read the truth table for this pin
    boolean row_match = true;
    for (int j = 0; j < minidin8_pin_count; j++)
    {
      // A tautology is, of course, already true
      if (i == j)
        continue;
      val = 1 - digitalRead(minidin8_pin_assignments[j]);
      if (debug)
      {
        sprintf(message, "pin(out), pin(in), column, table, read: %d, %s, %d, %s,%d, %d, %d", minidin8_pin_assignments[i], minidin8_pin_names[i], minidin8_pin_assignments[j], minidin8_pin_names[j], j, minidin8_truth_table[i][j], val);
        Serial.println(message);
      }
      if (val != minidin8_truth_table[i][j])
      {
        row_match = false;
        cable_good = false;
        if (!debug)
        {
          sprintf(message, "pin(out), pin(in), column, table, read: %d, %s, %d, %s,%d, %d, %d", minidin8_pin_assignments[i], minidin8_pin_names[i], minidin8_pin_assignments[j], minidin8_pin_names[j], j, minidin8_truth_table[i][j], val);
          // Serial.println(message);
        }
      }
      if (debug) delay(125);
    }
    if (debug)
    {
      sprintf(message, "Row %d match: %d", i, row_match);
      // Serial.println(message);
    }
    pinMode(minidin8_pin_assignments[i], INPUT_PULLUP);
  }
  if (debug)
  {
    sprintf(message, "isMiniDIN8() cable health: %d", cable_good);
    Serial.println(message);
  }
  return cable_good;
}

boolean isNewMiniDIN8()
{
  // Test MiniDIN8 cable

  // Innocent until proven guilty
  boolean cable_good = true;
  for (int i = 0; i < minidin8_pin_count; i++ )
  {
    if (debug)
    {
      sprintf(message, "Row %d, pin %s:", i, minidin8_pin_names[i]);
      Serial.println(message);
    }
    pinMode(minidin8_pin_assignments[i], OUTPUT);
    delay(1);  // Need to give the pin time to settle
    digitalWrite(minidin8_pin_assignments[i], LOW);  // We're using pullup, so invert logic
    // Read the truth table for this pin
    boolean row_match = true;
    for (int j = 0; j < minidin8_pin_count; j++)
    {
      // A tautology is, of course, already true
      if (i == j)
        continue;
      val = 1 - digitalRead(minidin8_pin_assignments[j]);
      if (debug)
      {
        sprintf(message, "pin(out), pin(in), column, table, read: %d, %s, %d, %s,%d, %d, %d", minidin8_pin_assignments[i], minidin8_pin_names[i], minidin8_pin_assignments[j], minidin8_pin_names[j], j, new_minidin8_truth_table[i][j], val);
        Serial.println(message);
      }
      if (val != new_minidin8_truth_table[i][j])
      {
        row_match = false;
        cable_good = false;
        if (!debug)
        {
          sprintf(message, "pin(out), pin(in), column, table, read: %d, %s, %d, %s,%d, %d, %d", minidin8_pin_assignments[i], minidin8_pin_names[i], minidin8_pin_assignments[j], minidin8_pin_names[j], j, new_minidin8_truth_table[i][j], val);
          // Serial.println(message);
        }
      }
      if (debug) delay(125);
    }
    if (debug)
    {
      sprintf(message, "Row %d match: %d", i, row_match);
      // Serial.println(message);
    }
    pinMode(minidin8_pin_assignments[i], INPUT_PULLUP);
  }
  if (debug)
  {
    sprintf(message, "isNewMiniDIN8() cable health: %d", cable_good);
    Serial.println(message);
  }
  return cable_good;
}

boolean isDIN5()
{
  // Test DIN5 cable

  // Innocent until proven guilty
  boolean cable_good = true;

  for (int i = 0; i < din5_pin_count; i++ )
  {
    if (debug)
    {
      sprintf(message, "Row %d, pin %s:", i, din5_pin_names[i]);
      Serial.println(message);
    }
    pinMode(din5_pin_assignments[i], OUTPUT);
    delay(1);  // Need to give the pin time to settle
    digitalWrite(din5_pin_assignments[i], LOW);  // We're using pullup, so invert logic
    // Read the truth table for this pin
    boolean row_match = true;
    for (int j = 0; j < din5_pin_count; j++)
    {
      // A tautology is, of course, already true
      if (i == j)
        continue;
      val = 1 - digitalRead(din5_pin_assignments[j]);
      if (debug)
      {
        sprintf(message, "pin(out), pin(in), column, table, read: %d, %s, %d, %s, %d, %d, %d", din5_pin_assignments[i], din5_pin_names[i], din5_pin_assignments[j], din5_pin_names[j], j, din5_truth_table[i][j], val);
        Serial.println(message);
      }
      if (val != din5_truth_table[i][j])
      {
        row_match = false;
        cable_good = false;
        if (!debug)
        {
          sprintf(message, "pin(out), pin(in), column, table, read: %d, %s, %d, %s, %d, %d, %d", din5_pin_assignments[i], din5_pin_names[i], din5_pin_assignments[j], din5_pin_names[j], j, din5_truth_table[i][j], val);
          // Serial.println(message);
        }
      }
      if (debug) delay(125);
    }
    if (debug)
    {
      sprintf(message, "Row %d match: %d", i, row_match);
      Serial.println(message);
    }
    pinMode(din5_pin_assignments[i], INPUT_PULLUP);
  }
  if (debug)
  {
    sprintf(message, "isDIN5() cable health: %d", cable_good);
    Serial.println(message);
  }
  return cable_good;
}
