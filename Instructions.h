#define ICount 8
const char* INames[ICount]=
{
{"$WB->$CI"},       //0
{"$WB->$HACK"},     //1
{"$WB->$SAFE"},     //2
{"$WB->$FULLPACK"}, //3
{"$WB->$DATA"},     //4
{"$WB->$RR"},       //5
{"$WB->$WR"},       //6
{"$WB->$INIT"}      //7
};
unsigned char *true_message="$WB->$true";
unsigned char *false_message="$WB->$false";
unsigned char *crc_error_message="$WB->$CRCERR";
unsigned char *error="$WB->$ERR";