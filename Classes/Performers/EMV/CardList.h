//
//  CardTypes.h
//  SmartCard
//
//  Created by Raúl Horacio Valencia Tenorio on 6/12/12.
//  Copyright (c) 2009 RHVT. All rights reserved.
//
//  Class Description: Constant definition of the cards and
//  commands that can be read
//
//	           (\___/)
//             (o\ /o)
//            /|:.V.:|\
//		      \\::::://
//        -----`"" ""`-----

#ifndef SmartCard_CardTypes_h
#define SmartCard_CardTypes_h

//Enumeration of the card types that can be read
typedef enum
{
    VISA_CREDIT_OR_DEBIT = 0,
    VISA_ELECTRON,
    MASTERCARD_CREDIT_OR_DEBIT,
    PSE_ONE_PAY_SYS_DDF01,
    CHINA_UNION_PAY_0,
    CHINA_UNION_PAY_1,
    CHINA_UNION_PAY_2,
    CHINA_UNION_PAY_3,
    V_PAY,
    VISA_PLUS,
    MASTERCARD_4,
    MAESTRO_DEBIT_CARD,
    CIRRUS_INTERBANK_NETWORK,
    MAESTRO_NETWORK,
    UK_DOMESTIC_MAESTRO_SOLO,
    AMERICAN_EXPRESS,
    DISCOVER,
    INTERAC_CANADA_DEBIT_CARD,
    JAPAN_CREDIT_BUREAU_LINK,
    UK_ATM_NETWORK_ATM_CARD,
    DANKORT_DENMARK_DEBIT_CARD,
    COGEBAN_ITALY_PAGOBANCOMAT,
    BANRISUL_BRAZIL_BANRICOMPRAS_DEBITO,
    ZKA_GERMANY_GIROCARDT,
    CB_CARD_FRANCE_1,
    CB_CARD_FRANCE_2,
    CARD_LIST_NUMBER
} CardListConstants;

//Enumeration of the SFI commands that contain information
//According to the EMV of Visa and Master Card
typedef enum
{
    SFI_01_REC_1 = 0,
    SFI_01_REC_2,
    SFI_01_REC_3,
    SFI_02_REC_1,
    SFI_02_REC_2,
    SFI_02_REC_3,
    SFI_02_REC_4,
    SFI_02_REC_5,
    SFI_02_REC_6,
    SFI_02_REC_7,
    SFI_02_REC_8,
    SFI_02_REC_9,
    SFI_03_REC_1,
    SFI_03_REC_2,
    SFI_03_REC_3,
    SFI_03_REC_4,
    SFI_04_REC_1,
    SFI_04_REC_2,
    SFI_04_REC_3,
    SFI_10_REC_4,
    SFI_10_REC_5,
    SFI_10_REC_6,
    SFI_LIST_NUMBER
} SFIListConstants;

//List of Tags that are available in a Visa or Mastercard
//smart card according to the EMV
typedef enum
{
    CARDHOLDER_NAME = 0,
    TRACK_2,
    APPLICATION_EXPIRATION_DATE,
    APP_PRIMARY_ACC_NUMBER,
    SERVICE_CODE,
//    TRANSACTION_CURRENCY_CODE,
    CARDHOLDER_VERIFICATION_METHOD_LIST,
    DDOL,// Dynamic Data Authentication Data Object List
    LANGUAGE_PREFERENCE,
    SIGNED_STATIC_APPLICATION_DATA,
    CERTIFICATION_AUTHORITY_PUBLIC_KEY_INDEX,
    ISSUER_PUBLIC_KEY_REMAINDER,
    ISSUER_PUBLIC_KEY_CERTIFICATE,
    APPLICATION_DISCRETIONARY_DATA,
    CARD_RISK_MANAGEMENT_DATA_OBJECT_LIST_1,
    CARD_RISK_MANAGEMENT_DATA_OBJECT_LIST_2,
    LOWER_CONSECUTIVE_OFFLINE_LIMIT,
    UPPER_CONSECUTIVE_OFFLINE_LIMIT,
    ISSUER_COUNTRY_CODE,
    ISSUER_ACTION_CODE_DEFAULT,
    ISSUER_ACTION_CODE_DENIAL,
    ISSUER_ACTION_CODE_ONLINE,
    APPLICATION_USAGE_CONTROL,
    STATIC_DATA_AUTHENTICATION_TAG_LIST,
    APPLICATION_CONCURRENCY_CODE,
    RESERVED_FOR_PAYMENT_SYSTEM_1,
    APPLICATION_VERSION_NUMBER,
    APPLICATION_CONCURRENCY_EXPONENT,
    RESERVED_FOR_PAYMENT_SYSTEM_2,
    PAN_SEQUENCE_NUMBER,
    APPLICATION_EFFECTIVE_DATE,
    TRACK_1_DATA,
    CARD_TAG_LIST_NUMBER
} CardTagListConstants;

//List of representations that a binary data could be transformed into
typedef enum
{
    STRING_REPRESENTATION = 0,
    DECIMAL_REPRESENTATION,
    HEX_REPRESENTATION,
    DATA_REPRESENTATION_NUMBER
} DataRepresentation;

class CardList
{
public:
    
//----------------------------------------
//	    CARD ID
//----------------------------------------
#pragma mark -
#pragma mark CARD ID

    //String literal of the card types
    const char *cardAIDStatement[CARD_LIST_NUMBER] =
    {
        "VISA_CREDIT_OR_DEBIT",
        "VISA_ELECTRON",
        "MASTERCARD_CREDIT_OR_DEBIT",
        "PSE_ONE_PAY_SYS_DDF01",
        "CHINA_UNION_PAY_0",
        "CHINA_UNION_PAY_1",
        "CHINA_UNION_PAY_2",
        "CHINA_UNION_PAY_3",
        "V_PAY",
        "VISA_PLUS",
        "MASTERCARD_4",
        "MAESTRO_DEBIT_CARD",
        "CIRRUS_INTERBANK_NETWORK",
        "MAESTRO_NETWORK",
        "UK_DOMESTIC_MAESTRO_SOLO",
        "AMERICAN_EXPRESS",
        "DISCOVER",
        "INTERAC_CANADA_DEBIT_CARD",
        "JAPAN_CREDIT_BUREAU_LINK",
        "UK_ATM_NETWORK_ATM_CARD",
        "DANKORT_DENMARK_DEBIT_CARD",
        "COGEBAN_ITALY_PAGOBANCOMAT",
        "BANRISUL_BRAZIL_BANRICOMPRAS_DEBITO",
        "ZKA_GERMANY_GIROCARDT",
        "CB_CARD_FRANCE_1",
        "CB_CARD_FRANCE_2",
    };
    
    //Commands of the card types
    const uint8_t cardAID[CARD_LIST_NUMBER][20] =
    {
        {0x00,0xa4,0x04,0x00,0x07,0xa0,0x00,0x00,0x00,0x03,0x10,0x10}, //VISA_CREDIT_OR_DEBIT
        {0x00,0xa4,0x04,0x00,0x07,0xa0,0x00,0x00,0x00,0x03,0x20,0x10}, //VISA_ELECTRON
        {0x00,0xa4,0x04,0x00,0x07,0xa0,0x00,0x00,0x00,0x04,0x10,0x10}, //MASTERCARD_CREDIT_OR_DEBIT
        {0x00,0xa4,0x04,0x00,0x0e,'1','P','A','Y','.','S','Y','S','.','D','D','F','0','1'}, //PSE_ONE_PAY_SYS_DDF01
        {0x00,0xa4,0x04,0x00,0x08,0xa0,0x00,0x00,0x03,0x33,0x01,0x01,0x01}, //CHINA_UNION_PAY_0
        {0x00,0xa4,0x04,0x00,0x08,0xa0,0x00,0x00,0x03,0x33,0x01,0x01,0x02}, //CHINA_UNION_PAY_1
        {0x00,0xa4,0x04,0x00,0x08,0xa0,0x00,0x00,0x03,0x33,0x01,0x01,0x03}, //CHINA_UNION_PAY_2
        {0x00,0xa4,0x04,0x00,0x08,0xa0,0x00,0x00,0x04,0x44,0x01,0x01,0x05}, //CHINA_UNION_PAY_3
        {0x00,0xa4,0x04,0x00,0x07,0xa0,0x00,0x00,0x00,0x03,0x20,0x20}, //V_PAY
        {0x00,0xa4,0x04,0x00,0x07,0xa0,0x00,0x00,0x00,0x03,0x80,0x10}, //VISA_PLUS
        {0x00,0xa4,0x04,0x00,0x07,0xa0,0x00,0x00,0x00,0x04,0x99,0x99}, //MASTERCARD_4
        {0x00,0xa4,0x04,0x00,0x07,0xa0,0x00,0x00,0x00,0x04,0x30,0x60}, //MAESTRO_DEBIT_CARD
        {0x00,0xa4,0x04,0x00,0x07,0xa0,0x00,0x00,0x00,0x04,0x60,0x00}, //CIRRUS_INTERBANK_NETWORK
        {0x00,0xa4,0x04,0x00,0x07,0xa0,0x00,0x00,0x00,0x05,0x00,0x01}, //MAESTRO_NETWORK
        {0x00,0xa4,0x04,0x00,0x07,0xa0,0x00,0x00,0x00,0x05,0x00,0x02}, //UK_DOMESTIC_MAESTRO_SOLO
        {0x00,0xa4,0x04,0x00,0x06,0xa0,0x00,0x00,0x00,0x25,0x01}, //AMERICAN_EXPRESS
        {0x00,0xa4,0x04,0x00,0x08,0xa0,0x00,0x00,0x00,0x01,0x52,0x30,0x10}, //DISCOVER
        {0x00,0xa4,0x04,0x00,0x07,0xa0,0x00,0x00,0x02,0x77,0x10,0x10}, //INTERAC_CANADA_DEBIT_CARD
        {0x00,0xa4,0x04,0x00,0x06,0xa0,0x00,0x00,0x00,0x65,0x10}, //JAPAN_CREDIT_BUREAU_LINK,
        {0x00,0xa4,0x04,0x00,0x07,0xa0,0x00,0x00,0x00,0x29,0x10,0x10}, //UK_ATM_NETWORK_ATM_CARD
        {0x00,0xa4,0x04,0x00,0x07,0xa0,0x00,0x00,0x01,0x21,0x10,0x10}, //DANKORT_DENMARK_DEBIT_CARD
        {0x00,0xa4,0x04,0x00,0x07,0xa0,0x00,0x00,0x00,0x14,0x00,0x01}, //COGEBAN_ITALY_PAGOBANCOMAT,
        
        {0x00,0xa4,0x04,0x00,0x07,0xa0,0x00,0x00,0x01,0x54,0x44,0x42}, //BANRISUL_BRAZIL
        {0x00,0xa4,0x04,0x00,0x0a,0xa0,0x00,0x00,0x03,0x59,0x10,0x10,0x02,0x80,0x01}, //ZKA_GERMANY_GIROCARDT
        {0x00,0xa4,0x04,0x00,0x07,0xa0,0x00,0x00,0x00,0x42,0x10,0x10}, //CB_CARD_FRANCE_1
        {0x00,0xa4,0x04,0x00,0x0f,0xA0,0x00,0x00,0x03,0x33,0x43,0x55,0x50,0x2d,0x4d,0x4f,0x42,0x49,0x4c,0x45}, //CB_CARD_FRANCE_2
        
    };
    
    //Size of each command card type
    const uint8_t cardAIDSize[CARD_LIST_NUMBER] =
    {
        12, 12, 12, 19, 13, 13, 13, 13, 12, 12, 12, 12, 12, 12,
        12, 11, 13, 12, 11, 12, 12, 12, 12, 15, 12, 20
    };
    
//----------------------------------------
//	SFI COMMAND
//----------------------------------------
#pragma mark -
#pragma mark SFI COMMAND
    
    //Hexadecimal representation of the SFI commands that contain valuable information
    const uint8_t sfiCommand[CARD_LIST_NUMBER][20] =
    {
        {0x00,0xb2,0x01,0x0C,0x00}, //SFI 01 REC 1,
        {0x00,0xb2,0x02,0x0C,0x00}, //SFI 01 REC 2
        {0x00,0xb2,0x03,0x0C,0x00}, //SFI 01 REC 3
        {0x00,0xb2,0x01,0x14,0x00}, //SFI 02 REC 1
        {0x00,0xb2,0x02,0x14,0x00}, //SFI 02 REC 2
        {0x00,0xb2,0x03,0x14,0x00}, //SFI 02 REC 3
        {0x00,0xb2,0x04,0x14,0x00}, //SFI 02 REC 4
        {0x00,0xb2,0x05,0x14,0x00}, //SFI 02 REC 5
        {0x00,0xb2,0x06,0x14,0x00}, //SFI 02 REC 6
        {0x00,0xb2,0x07,0x14,0x00}, //SFI 02 REC 7
        {0x00,0xb2,0x08,0x14,0x00}, //SFI 02 REC 8
        {0x00,0xb2,0x09,0x14,0x00}, //SFI 02 REC 9
        {0x00,0xb2,0x01,0x1C,0x00}, //SFI 03 REC 1
        {0x00,0xb2,0x02,0x1C,0x00}, //SFI 03 REC 2
        {0x00,0xb2,0x03,0x1C,0x00}, //SFI 03 REC 3
        {0x00,0xb2,0x04,0x1C,0x00}, //SFI 03 REC 4
        {0x00,0xb2,0x01,0x24,0x00}, //SFI 04 REC 1
        {0x00,0xb2,0x02,0x24,0x00}, //SFI 04 REC 2
        {0x00,0xb2,0x03,0x24,0x00}, //SFI 04 REC 3
        {0x00,0xb2,0x04,0x54,0x00}, //SFI 10 REC 4
        {0x00,0xb2,0x05,0x54,0x00}, //SFI 10 REC 5
        {0x00,0xb2,0x06,0x54,0x00}  //SFI 10 REC 6
    };
    
    const uint8_t sfiSize = 5;
    
//----------------------------------------
//	CARD TAG
//----------------------------------------
#pragma mark -
#pragma mark CARD TAG
    
    //Tags literal string according to EMV
    const char *cardTagStatement[CARD_TAG_LIST_NUMBER] =
    {
        "CARDHOLDER_NAME"
        "TRACK_2",
        "APPLICATION_EXPIRATION_DATE",
        "APP_PRIMARY_ACC_NUMBER",
        "SERVICE_CODE",
        //    TRANSACTION_CURRENCY_CODE",
        "CARDHOLDER_VERIFICATION_METHOD_LIST",
        "DDOL",  // Dynamic Data Authentication Data Object List
        "LANGUAGE_PREFERENCE",
        "SIGNED_STATIC_APPLICATION_DATA",
        "CERTIFICATION_AUTHORITY_PUBLIC_KEY_INDEX",
        "ISSUER_PUBLIC_KEY_REMAINDER",
        "ISSUER_PUBLIC_KEY_CERTIFICATE",
        "APPLICATION_DISCRETIONARY_DATA",
        "CARD_RISK_MANAGEMENT_DATA_OBJECT_LIST_1",
        "CARD_RISK_MANAGEMENT_DATA_OBJECT_LIST_2",
        "LOWER_CONSECUTIVE_OFFLINE_LIMIT",
        "UPPER_CONSECUTIVE_OFFLINE_LIMIT",
        "ISSUER_COUNTRY_CODE",
        "ISSUER_ACTION_CODE_DEFAULT",
        "ISSUER_ACTION_CODE_DENIAL",
        "ISSUER_ACTION_CODE_ONLINE",
        "APPLICATION_USAGE_CONTROL",
        "STATIC_DATA_AUTHENTICATION_TAG_LIST",
        "APPLICATION_CONCURRENCY_CODE",
        "RESERVED_FOR_PAYMENT_SYSTEM_1",
        "APPLICATION_VERSION_NUMBER",
        "APPLICATION_CONCURRENCY_EXPONENT",
        "RESERVED_FOR_PAYMENT_SYSTEM_2",
        "PAN_SEQUENCE_NUMBER",
        "APPLICATION_EFFECTIVE_DATE",
        "TRACK_1_DATA"
    };
    
    //Tag list of important data from EMV
    const uint8_t cardTags[CARD_TAG_LIST_NUMBER][2] = 
    {
        {0x5F,0x20},    //CARDHOLDER_NAME
        {0x57},         //TRACK_2 DATA
        {0x5F,0x24},    //APPLICATION_EXPIRATION_DATE
        {0x5A},         //APP_PRIMARY_ACC_NUMBER - PAN
        {0x5F,0x30},    //SERVICE_CODE
//        {0x5F,0x2A},    //TRANSACTION_CURRENCY_CODE
        {0x8E},         //CARDHOLDER_VERIFICATION_METHOD_LIST,
        {0x9F,0x49},    //DDOL - Dynamic Data Authentication Data Object List
        {0x5F,0x2D},    //LANGUAGE_PREFERENCE
        {0x93},         //SIGNED_STATIC_APPLICATION_DATA
        {0x8F},         //CERTIFICATION_AUTHORITY_PUBLIC_KEY_INDEX
        {0x92},         //ISSUER_PUBLIC_KEY_REMAINDER
        {0x90},         //ISSUER_PUBLIC_KEY_CERTIFICATE
        {0x9F,0x05},    //APPLICATION_DISCRETIONARY_DATA
        {0x8C},         //CARD_RISK_MANAGEMENT_DATA_OBJECT_LIST_1
        {0x8D},         //CARD_RISK_MANAGEMENT_DATA_OBJECT_LIST_2
        {0x9F,0x14},    //LOWER_CONSECUTIVE_OFFLINE_LIMIT
        {0x9F,0x23},    //UPPER_CONSECUTIVE_OFFLINE_LIMIT
        {0x5F,0x28},    //ISSUER_COUNTRY_CODE
        {0x9F,0x0D},    //ISSUER_ACTION_CODE_DEFAULT
        {0x9F,0x0E},    //ISSUER_ACTION_CODE_DENIAL
        {0x9F,0x0F},    //ISSUER_ACTION_CODE_ONLINE
        {0x9F,0x07},    //APPLICATION_USAGE_CONTROL
        {0x9F,0x4A},    //STATIC_DATA_AUTHENTICATION_TAG_LIST
        {0x9F,0x42},    //APPLICATION_CONCURRENCY_CODE
        {0x9F,0x51},    //RESERVED_FOR_PAYMENT_SYSTEM_1
        {0x9F,0x08},    //APPLICATION_VERSION_NUMBER
        {0x9F,0x44},    //APPLICATION_CONCURRENCY_EXPONENT
        {0x9F,0x57},    //RESERVED_FOR_PAYMENT_SYSTEM_2
        {0x5F,0x30},    //PAN_SEQUENCE_NUMBER
        {0x5F,0x25},    //APPLICATION_EFFECTIVE_DATE
        {0x9F,0x1F},    //TRACK_1_DATA
    };
    
    const uint8_t cardTagsSize[CARD_TAG_LIST_NUMBER] =
    {
        2, 1, 2, 1, 2, 1, 2, 2, 1, 1, 1, 1, 2, 1, 1,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
    };
};

#endif
