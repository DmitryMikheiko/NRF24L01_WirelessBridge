#define ICount 8
    flash const char* INames[ICount]= {
    {"$WB->$CI"},       //0
    {"$WB->$HACK"},     //1
    {"$WB->$SAFE"},     //2
    {"$WB->$FULLPACK"}, //3
    {"$WB->$DATA"},     //4
    {"$WB->$RR"},       //5
    {"$WB->$WR"},       //6
    {"$WB->$INIT"}      //7
};

flash const char *true_message="$WB->$true";
flash const char *false_message="$WB->$false";
flash const char *crc_error_message="$WB->$CRCERR";
//const char *error="$WB->$ERR";  
flash const char *uart_error = "$WB->$UARTERR";
flash const char *tx_error = "$WB->$TXERROR";