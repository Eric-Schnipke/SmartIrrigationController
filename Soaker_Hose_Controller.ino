int ValvePin = 4;                // Solenoid valve connected to pin 4
int LEDPin = 13;
int DepthPin = 5;

void setup()                    // run once, when the sketch starts
{
  pinMode(ValvePin, OUTPUT);      // sets the digital pin as output
  pinMode(LEDPin, OUTPUT);
  Serial.begin(9600);
}

//===========Classes=====================//
class DispenseProfile{
  //the dispense profile is the way that the barrel network dispenses water.  If the profile is "increasing", the profile distirbutes very little water during the first cycles after a rain and then increases the amount as time goes on after a rain.  "Decreasing" profiles behave oppositely.  "Equivalent" profiles dispense water equivalently throughout all cycles.
public:
    
    void calculateDispenseProfile(double totalWaterHeight, int numberOfCycles){
        //totalWaterHeight is the height of the water in the barrel network.
        //numberOfCycles is the number of cycles to make the water last.
        
        lastWaterHeightRecord = totalWaterHeight; //saving initial water height in dispensing profile.
        
        int avgPercent = 1/numberOfCycles; //if a histogram of all of the watering cycle were created with dispense percentage on the y-axis and cycle number on the x-axis, the sumation of all of the bars would be 100%.  "avgPercent" is the average height of all of the bars.

        double lastDispensePercent = 2*avgPercent - initialDispensePercent; //percent of water height to dispense on the last cycle.
        slope = (lastDispensePercent - initialDispensePercent)/(numberOfCycles - 1);
    }
    
    double getDispenseHeight(int cycleNumber){
        return (slope*cycleNumber+initialDispensePercent)*lastWaterHeightRecord; //returns depth to dispense until in depth units.
    }
    
    
private:
    double lastWaterHeightRecord; //last recorded water height.  Only refreshed whenever calculateDispenseProfile() is called.
    double slope; //slope of water dispensing profile.
    double const static initialDispensePercent = 0; //percent of water height to dispense on the first cycle.  For "increasing" watering profiles this must be set to less than "avgPercent", for "decreasing" watering profiles this must be set to greater than "avgPercent", and for "equivalent" watering profiles this must be set to equal "avgPercent".
};

//===========Functions=====================//

void openValve(){
  //open valve...
  digitalWrite(ValvePin, HIGH);
  digitalWrite(LEDPin,HIGH);
  Serial.println("Valve Open");
}  

void closeValve(){
  //close valve...
  digitalWrite(ValvePin, LOW);
  digitalWrite(LEDPin,LOW);
  Serial.println("Valve closed");  
}  

void dispenseWater(double dispenseToHeight){
  //dispenseToHeight is the water height to dispense to.
  double depth = readWaterHeight();
  
  Serial.print("Dispensing to ");
  Serial.println(dispenseToHeight);
  
  openValve();
  
  while(depth > dispenseToHeight){
    //occupy system time until the water height readings match the water height goal... 
    Serial.print("Depth is ");
    Serial.println(depth);
    
    depth = readWaterHeight();
    delay(100); //delay 0.1 seconds to allow the water height to react.
  }
  
  closeValve();
}

double readWaterHeight(){
  //return the current height of the water in the barrel network.
  
  return analogRead(DepthPin);
}  
  
//=================Main Loop===================//
void loop()
{
    int daysToMakeWaterLast = 7; //measured in days
    int wateringFrequency = 3; //delay between watering cycles, measured in hours.

    double currentWaterHeight = 0; //current height of the water in the barrel network.
    double prevWaterHeight = 0; //previous height of the water in the barrel network.
    int cycleNumber = 0; //current water dispensing cycle number counted from the last time it rained.
    
    //create new water dispensing profile.
    DispenseProfile wateringProfile;
    
    //calculate number of watering cycles.
    int numWaterCycles = daysToMakeWaterLast*24*(1/wateringFrequency);
    
    //calculate initial watering profile.
    currentWaterHeight = readWaterHeight();
    wateringProfile.calculateDispenseProfile(currentWaterHeight, numWaterCycles);
    
    //loop through watering cycles.
    while(1){
        
        //read height of water in barrel network.
        currentWaterHeight = readWaterHeight();
        
        //if barrel height is heigher than the last recorded height, it rained. A new dispensing profile should be calculated and watering cycles should restart.
        if(currentWaterHeight > prevWaterHeight){
            Serial.println("It rained.  Calculating new water dispensing profile and resetting cycle count...");
            wateringProfile.calculateDispenseProfile(currentWaterHeight, numWaterCycles);
            cycleNumber = 0;
        }
        
        //dispense appropriate amount of water in accordance with dispensing profile and water cycle index.
        dispenseWater(currentWaterHeight - wateringProfile.getDispenseHeight(cycleNumber));
        
        //increment water cycle counter.
        cycleNumber++;
        
        //save current water height for next cycle.
        prevWaterHeight = currentWaterHeight;
        
        delay(wateringFrequency*3600*1000);
    }

}
