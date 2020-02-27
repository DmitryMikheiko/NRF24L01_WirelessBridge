#define ICount 8
    const char* INames[ICount]= {
    {"$WB->$CI"},       //0
    {"$WB->$HACK"},     //1
    {"$WB->$SAFE"},     //2
    {"$WB->$FULLPACK"}, //3
    {"$WB->$DATA"},     //4
    {"$WB->$RR"},       //5
    {"$WB->$WR"},       //6
    {"$WB->$INIT"}      //7
};

const char *true_message="$WB->$true";
const char *false_message="$WB->$false";
const char *crc_error_message="$WB->$CRCERR";
const char *error="$WB->$ERR";  
const char *uart_error = "$WB->$UARTERR";
const char *tx_error = "$WB->$TXERROR";