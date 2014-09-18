/* Gowri Somanath,
 2014
Arduino code for using Pulse Heart sensor for Intel Galileo/Edison
See the sketch for usage.
*/

#define PULSE_N_MAX 200
#define MAX_PULSE_UP 20

    void setupPulse(int ppin,int writeToFile);
    void setupPulse2(int ppin,int writeToFile,int checkpulse_interval);
    void checkPulse();
    int getPulseRate();


    
int pulsepin=0;
int pulse_N=0;
int pulse_x[PULSE_N_MAX];
int pulse_file_cnt=0;
int pulse_FILE_MAX=0;

boolean PULSE_UP=false;int pulse_up_cnt=0;
boolean PULSE_DOWN=false;int pulse_down_cnt=0;
int pulse_cnt=0;
int pulse_up_time[MAX_PULSE_UP];

int pulse_rate=0;

void getHeart();
void calculatePulseRate();
void writePulseData(int fileNum);


/*Create a timer object called timer_obj and
 make sure to call timer_obj.update() in loop
 if you want to write the pulses into files write >0 to writeToFile*/
/*trying to use a timer to //ly manage multiple sensor inputs was getting slow for Edison IDE. so better use the setupPulse2 method which samples in a loop and calculate*/
#define PULSE_TIME_GAP 2 //only relevant for SETUP not SETUP2
/*void setupPulse(int ppin,int writeToFile)
 {
 #define SETUP 1
 pulse_file_cnt=0;
 pulse_FILE_MAX=writeToFile;
 pulsepin=ppin;
 pulse_N=0;
 pulse_cnt=0;
 //  digitalWrite(13,HIGH);
 int t=timer_obj.every(PULSE_TIME_GAP,getHeart,PULSE_N_MAX-1);
 }//setup*/

void setupPulse2(int ppin,int writeToFile,int checkpulse_interval)
{
#define SETUP2 1
    pulse_file_cnt=0;
    pulse_FILE_MAX=writeToFile;
    pulsepin=ppin;
    pulse_N=0;
    pulse_cnt=0;
    pulse_up_cnt=0;
    pulse_down_cnt=0;
    digitalWrite(13,HIGH);
    timer_obj.every(checkpulse_interval,checkPulse);
    checkPulse();
    
}//setup

void checkPulse()
{
    pulse_cnt=0;
    pulse_N=0;
    digitalWrite(13,HIGH);
    /* for(int i=0;i<PULSE_N_MAX;i++)
     {
     getHeart();
     }*/
    timer_obj.every(1,getHeart,PULSE_N_MAX);
    digitalWrite(13,LOW);
}

int getPulseRate()
{
    return pulse_rate;
}//getPulseRate

void doHeartRateAgain()
{
    pulse_cnt=0;
    pulse_N=0;
#ifdef SETUP
    timer_obj.every(PULSE_TIME_GAP,getHeart,PULSE_N_MAX-1);
#endif
#ifdef SETUP2
    checkPulse();
#endif
    
    
}

void getHeart()
{
    
    pulse_x[pulse_N]=analogRead(pulsepin);
    
    if(pulse_x[pulse_N]<510) {
        PULSE_DOWN=true;pulse_down_cnt++;
        PULSE_UP=false;pulse_up_cnt=0;
    }
    //its just some noise with down-spike
    if(PULSE_DOWN==true && pulse_down_cnt<3 && pulse_x[pulse_N]>510)
    {
        pulse_down_cnt=0;pulse_up_cnt=0;
        PULSE_DOWN=false;
    }
    if(pulse_cnt<MAX_PULSE_UP && PULSE_DOWN==true && pulse_down_cnt>3 && pulse_x[pulse_N]>550)
    {
        PULSE_UP=true;//pulse_up_cnt++;
        PULSE_DOWN=false;pulse_down_cnt=0;
        pulse_up_time[pulse_cnt]=millis();//pulse_N;
        pulse_cnt++;
        
        
    }
    pulse_N++;
    if(pulse_N==PULSE_N_MAX-1)
    {
        if(pulse_FILE_MAX>0 && pulse_file_cnt<pulse_FILE_MAX)
            writePulseData(pulse_file_cnt);
        else
        {
            calculatePulseRate();
#ifdef SETUP
            timer_obj.every(PULSE_TIME_GAP,getHeart,PULSE_N_MAX-1);
#endif
            
        }
    }
    
}//getHeart

//calclate pulse rate as average time between pulses
void calculatePulseRate()
{
    //digitalWrite(13,LOW);
    int i,psum=0,maxp=0,tmp,meanp=0,cnt=0;
    //get the mean
    for( i=0;i<pulse_cnt-1;i++)
    {
        tmp=(pulse_up_time[i+1]-pulse_up_time[i]);
        psum=psum+tmp;
       // if(tmp>maxp) maxp=tmp;
        
    }
    psum=psum/i;
    
    //eliminate noise based on mean
    maxp=0;
    for( i=0;i<pulse_cnt-1;i++)
    {
        tmp=(pulse_up_time[i+1]-pulse_up_time[i]);
        if(tmp<1.2*psum && tmp>0.75*psum)
        {
         if(tmp>maxp) maxp=tmp;
            meanp+=tmp;
            cnt++;
        }
        
    }
    if(cnt>1) meanp=meanp/(cnt-1);
    // pulse_rate=(psum*PULSE_TIME_GAP)/(pulse_cnt-2);
    //to get the beats per minute
    pulse_rate=60000/meanp;
    
    String cmd;
    char cmdc[50];
    cmd="echo ";
    cmd+= pulse_rate;
    cmd+=" , $(date) >> pulse_rate.txt";
    cmd.toCharArray(cmdc,cmd.length()+2);
    system(cmdc);
    
    
    pulse_cnt=0;
    pulse_N=0;
    
}//calculatePulseRate



void writePulseData(int fileNum)
{
    //digitalWrite(13,LOW);
    String cmd;
    char cmdc[50];
    
    cmd="echo  > heart";
    cmd+=pulse_file_cnt;
    cmd+=".txt";
    cmd.toCharArray(cmdc,cmd.length()+2);
    system(cmdc);
    
    cmd="echo  ";
    // cmd+=pulse_cnt;
    cmd+=" > pulse";
    cmd+=pulse_file_cnt;
    cmd+=".txt";
    cmd.toCharArray(cmdc,cmd.length()+2);
    system(cmdc);
    
    digitalWrite(13,LOW);
    //write out raw values
    for(int i=0;i<pulse_N;i++)
    {
        cmd="echo ";
        cmd+=pulse_x[i];
        cmd+=" >> heart";
        cmd+=pulse_file_cnt;
        cmd+=".txt";
        cmd.toCharArray(cmdc,cmd.length()+2);
        system(cmdc);
    }
    
    //calculate pulse rate and write out the "pulse"
    if(pulse_cnt>2)
    {
        int i,psum=0,maxp=0,tmp,meanp=0,cnt=0;
        for( i=0;i<pulse_cnt-1;i++)
        {
            tmp=(pulse_up_time[i+1]-pulse_up_time[i]);
            psum=psum+tmp;
            //if(tmp>maxp) maxp=tmp;
            cmd="echo ";
            cmd+=pulse_up_time[i];
            cmd+=",";
            cmd+=tmp;
            cmd+=" >> pulse";
            cmd+=pulse_file_cnt;
            cmd+=".txt";
            cmd.toCharArray(cmdc,cmd.length()+2);
            system(cmdc);
        }
        psum=psum/i;
        maxp=0;
        for( i=0;i<pulse_cnt-1;i++)
        {
            tmp=(pulse_up_time[i+1]-pulse_up_time[i]);
            if(tmp<1.5*psum && tmp>0.5*psum)
            {
                if(tmp>maxp) maxp=tmp;
                meanp+=tmp;
                cnt++;
                
                cmd="echo ";
                cmd+=pulse_up_time[i];
                cmd+=",";
                cmd+=tmp;
                cmd+=" >> pulseUsed";
                cmd+=pulse_file_cnt;
                cmd+=".txt";
                cmd.toCharArray(cmdc,cmd.length()+2);
                system(cmdc);

            }
            
        }
        
        
        if(cnt>1) meanp=meanp/(cnt-1);
        // pulse_rate=(psum*PULSE_TIME_GAP)/(pulse_cnt-2);
        //to get the beats per minute
        pulse_rate=60000/meanp;
        
        
        cmd="echo ";
        cmd+= pulse_rate;
        cmd+=" > pulse_rate";
        cmd+=pulse_file_cnt;
        cmd+=".txt";
        cmd.toCharArray(cmdc,cmd.length()+2);
        system(cmdc);
    }
    else
    {
        system("echo 0 >no_pulse.txt");
    }
    
    pulse_N=0;
    pulse_file_cnt++;
    if(pulse_file_cnt<pulse_FILE_MAX)
    {
        pulse_N=0;
        pulse_cnt=0;
        //digitalWrite(13,HIGH);
#ifdef SETUP
        timer_obj.every(PULSE_TIME_GAP,getHeart,PULSE_N_MAX-1);
#endif
        
    }
    
}//writeAccelData



