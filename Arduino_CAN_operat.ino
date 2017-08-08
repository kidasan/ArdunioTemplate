//-----------------------------------------------
// Arduino CAN制御プログラム.
//
//
//-----------------------------------------------
//

//-----------------------------------------------
// 使用方法
//   D-SUBにCAN-H, CAN-L信号を繋いでボードを
//   実行する。
//   D-SUBの端子の仕様はOBD-IIではないので注意。
//
//   o(1) o(2) o(3) o(4) o(5)
//      o(6) o(7) o(8) o(9)
//
//   CAN-H : 3pin,  CAN-L : 5pin
//-----------------------------------------------

#include <mcp_can.h>
#include <SPI.h>

// CAN_CSはARDUNIOのサンプルでは10ピンだが基板のデフォルトでは9ピンとなる。
// 違っている場合は、初期化エラーとなる。
#define     CAN_CS      9   // 回路図上では9ピン扱い
#define     CAN_INT     2   // 回路図上では2ピン扱い

MCP_CAN     CAN0( CAN_CS );

//-----------------------------------------------
// setup
// @brief このセットアップはブート時に呼ばれる。
//  @param none
//  @return none
//-----------------------------------------------
void    setup()
{
    // USBポートをシリアルコンソールとして利用する。
    // 書き込みポートなのらプログラムを書き直す場合には
    // シリアルコンソールの端末を閉じる事。
    Serial.begin(115200);

    // MCP2515の初期化。
    // ボーレート 500Kbps, CAN IDのフィルタやマスクはしない。
    if (CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) == CAN_OK) {
        Serial.println("MCP2515 Initialized Successfully!");
    }
    else {
        Serial.println("Error Initializing MCP2515...");
    }

    // ノーマルモードで利用する。
    // ACK返送は自動。
    CAN0.setMode(MCP_NORMAL);


    // CANの割り込みピン定義
    pinMode(CAN_INT, INPUT);

    Serial.println("Setup completed.");
}

// [variables]

#define MY_CANID        104 // CAN-ID: ボードによって適切に設定する。

#define STANDARD_FRAME  0x80000000
#define REMOTE_FRAME    0x40000000

long unsigned int   txId = MY_CANID;    // 32bit整数だけ型が独特
unsigned char       txBuf[8];
long unsigned int   rxId;
unsigned char       rxBuf[8];
unsigned char       length;
unsigned char       sendStatus = CAN_OK;

//
#define ANALOG_PORT A0
int analogInput;

//-----------------------------------------------
// loop()
// @brief アプリケーションコード
//  @param none
//  @return none
// @note これはwhlieで回さなくても常に呼び出される。
//-----------------------------------------------
void    loop()
{
    delay(10);      // CAN制御のインターバル(10ms).

    // CANフレームの受信
    if (!digitalRead(CAN_INT)) {
        CAN0.readMsgBuf(&rxId, &length, rxBuf);
        if ((rxId & STANDARD_FRAME) == STANDARD_FRAME) {
            Serial.println("receive STANDARD frame");
        }
        if ((rxId & REMOTE_FRAME) == REMOTE_FRAME) {
            Serial.println("receive REMOTE frame");
        }
    }

    // センサーからの入力
    analogInput = analogRead(ANALOG_PORT);

    // CANメッセージの組み立て.
    txBuf[0] = (unsigned char)(analogInput&0xFF00) >> 8;
    txBuf[1] = (unsigned char)(analogInput&0xFF);
    length = 2;

    // CANメッセージの送信
    sendStatus = CAN0.sendMsgBuf(txId, 0, length, txBuf);
    if (sendStatus == CAN_OK) {
        Serial.println("Message Sent Successfully");
    }
    else {
        Serial.println("Error Sending Message");
    }

}

