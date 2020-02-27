#include "Instructions.h"
//unsigned char LastILength =0;
extern volatile bool HackMode;
extern volatile bool SafeMode;
extern volatile bool WaitData;
extern volatile bool ID_wait;
extern volatile bool FullPack;
extern volatile bool RR_WaitData;
extern volatile bool WR_WaitData;
unsigned char ID[5];
unsigned char pos;
unsigned char value;
unsigned char FindInstruction(unsigned char *data,unsigned char length)
{
 unsigned char n;
 bool command_find=false;     
 if(SafeMode)
 { for(n=0;n<ICount;n++)
  { 
   if(length==strlen(INames[n]))
   {      
    if((strcmp(data,INames[n]))==0) return n; 
   }    
  }                
 } 
 else 
 { 
   if(data[0]=='$')
   {  
     for(n=0;n<ICount;n++)  
     {
     command_find=true;
     for(pos=0;(pos<strlen(INames[n])) && (pos<length) ;pos++)
      {
      if(INames[n][pos]!=data[pos])
       {
        command_find=false; break;
       }  
      }
      if(command_find && (pos==strlen(INames[n])))return n;
     }
     
   }
 }
 return 255;
}
void IProcc(unsigned char num,unsigned char *data)
{
switch (num)
{
  case 0:                         //Change ID      
     if(SafeMode){ WaitData=true;ID_wait=true;  }
     else  
     { 
      pos=strlen(INames[0]);
      ID[0]=data[pos++];
      ID[1]=data[pos++];
      ID[2]=data[pos++];
      ID[3]=data[pos++];
      ID[4]=data[pos];  
      SetID(ID);
      printf("%s",true_message);
     }
  break;
  
  case 1:                         //Hack
       if(HackMode)
       {
        HackMode=false; 
        NRF24L01_hack_mode(HackMode);
        printf("%s",false_message);
       }  
       else 
       {
        HackMode=true;    
        NRF24L01_hack_mode(HackMode);
        printf("%s",true_message);
       }
  break;   
    
  case 2:                         //Safe    
       if(!FullPack)
       {
        if(SafeMode)
        { 
         SafeMode=false;printf("%s",false_message);
        }   
        else 
        {           
         SafeMode=true; printf("%s",true_message);
        } 
       } 
       else
       {
        printf("%s",false_message); 
       }
  break;
    
  case 3:                         //FULLPACK 
       if(!SafeMode)
       {
        if(FullPack)
        {
         FullPack=false;
         printf("%s",false_message);
        }
        else 
        {
         FullPack=true;
         printf("%s",true_message);          
        }
       }
       else 
       {
          printf("%s",false_message);
       }
  break;
    
  case 4:                         //Data  
     if(SafeMode)
     {     
       WaitData=true;      
     }
  break;
    
  case 5:                     
     if(SafeMode)
     {    
     if(!RR_WaitData)
     {
      RR_WaitData=true; 
      WaitData=true;  
     }  
      else 
      {
        RR_WaitData=false;  
        value=NRF24L01_ReadRigester(data[1]);
        putchar(value);
      }
     }                
     else 
     {
      value=NRF24L01_ReadRigester(data[strlen(INames[5])]);
      putchar(value);
     }   
  break;     
  
  case 6:                     
      if(SafeMode)
     {    
      if(!WR_WaitData)
      {
       WR_WaitData=true; 
       WaitData=true;  
      }  
      else 
      {
        WR_WaitData=false;  
        NRF24L01_WriteRigester(data[1],data[2]); 
        printf("%s",true_message);
      }
     }                
     else 
     {
       NRF24L01_WriteRigester(data[strlen(INames[5])],data[strlen(INames[5])+1]); 
       printf("%s",true_message);
     }  
  break;
  
  case 7:                     
    NRF24L01_init(); 
    printf("%s",true_message);
    
  break;
  
  case 8:                     
   
  break;
  
  default: break;
};
}