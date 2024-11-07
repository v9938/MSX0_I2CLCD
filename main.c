////////////////////////////////////////////////////////////////////
//	MSX0 Sample program : I2C Character LCD
//	Copyright @v9938
//	23/12/01 Ver 1.0 	初版
////////////////////////////////////////////////////////////////////

// 秋月電子扱いのI2C LCD AQM0802Aをつかった
// MSX0 サンプルプログラムです。
// Cコンパイラーはz88dk用です。
// I2Cのコードは、libiot.cの一部関数を参照にさせていただいています。

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
// #include "libiot.h"

// 秋月電子扱いのAQM0802A
// https://akizukidenshi.com/catalog/g/gK-06795/

// MSX0における、I2Cのデバイスパス
#define I2CADDR_LCD "device/i2c_a/3E"

// debug IOT print
// #define DEBUGEMU

#ifdef DEBUGEMU
// MSX実機でデバッグするときに使う
#define IOT_PORT " O:0x%02x"

// MSX実機ではIOTで使っているIOが使えないので画面に出力する
#define IOT_OUTP printf
#define IOT_INP dummyInp

unsigned char dummyInp(unsigned char *add)
{
    // Debug用のDummy関数
    printf(" I:0x00\n");
    return 0;
}

#else
// MSX0 IOT命令を使う場合の定義

// IOT命令で使われるIOポート
#define IOT_PORT 8

// MSX0ではoutp/inpを使う
#define IOT_OUTP outp
#define IOT_INP inp
#endif

void wait(int waitnum)
{
    // Wait関数：jiffyを使って、16ms x NのWaitを行う関数

    unsigned char tmp;
    unsigned char *jiffy;

    jiffy = (unsigned char *)0xfc9e; // JIFFY
    tmp = *jiffy;
    tmp = tmp + (unsigned char)waitnum;
    if (*jiffy > tmp)
        while (*jiffy < 0x80)
            ;

    while (*jiffy < tmp)
        ;
}
int iotfindi_(const char *device_path)
{
    outp(IOT_PORT, 0xE0);
    outp(IOT_PORT, 0x01);
    outp(IOT_PORT, 0x53);
    outp(IOT_PORT, 0xC0);
    int l = strlen(device_path);
    outp(IOT_PORT, l);
    for (int i = 0; i < l; i++)
    {
        outp(IOT_PORT, *device_path);
        device_path++;
    }
    outp(IOT_PORT, 0x00);
    unsigned char r[2];
    r[0] = inp(IOT_PORT);
    outp(IOT_PORT, 0xE0);
    outp(IOT_PORT, 0x01);
    outp(IOT_PORT, 0x11);
    outp(IOT_PORT, 0x80);
    r[0] = inp(IOT_PORT);
    r[0] = inp(IOT_PORT);
    r[1] = inp(IOT_PORT);
    return *((int *)r);
}

int iotfinds_(const char *device_path, char **result, int num)
{
    outp(IOT_PORT, 0xE0);
    outp(IOT_PORT, 0x01);
    outp(IOT_PORT, 0x53);
    outp(IOT_PORT, 0xC0);
    int l = strlen(device_path);
    outp(IOT_PORT, l);
    for (int i = 0; i < l; i++)
    {
        outp(IOT_PORT, *device_path);
        device_path++;
    }
    outp(IOT_PORT, 0x00);
    int ret = inp(IOT_PORT);
    outp(IOT_PORT, 0xE0);
    outp(IOT_PORT, 0x01);
    outp(IOT_PORT, 0x13);
    outp(IOT_PORT, 0x80);
    for (int i = 0; i < num; i++)
    {
        l = inp(IOT_PORT);
        for (int j = 0; j < l; j++)
        {
            result[i][j] = inp(IOT_PORT);
        }
        result[i][l] = 0;
    }
    return 0;
}
char iotputs_n(const char *device_path, char *value, int len)
{
    // I2C出力関数：I2Cにlenのサイズ分データを出力する
    int l;
    unsigned char i;
    unsigned char r;

    // MSX0 I2C Device Pathを送信
    l = strlen(device_path);

    IOT_OUTP(IOT_PORT, 0xE0);
    IOT_OUTP(IOT_PORT, 0x01);
    IOT_OUTP(IOT_PORT, 0x53); // Device Path指定
    IOT_OUTP(IOT_PORT, 0xC0); // 引数送信指定
    IOT_OUTP(IOT_PORT, l);    // 引数文字数
    for (i = 0; i <= l; i++)
    {
        IOT_OUTP(IOT_PORT, *device_path);
        device_path++;
    }
    r = IOT_INP(IOT_PORT); // Pathが無い場合は0以外が返る
    if (r != 0x00)
    {
        printf("Error:Device Path not found E:%0x\n", r);
        return r; // Errorなので戻る
    }

    // MSX0 I2C送信
    IOT_OUTP(IOT_PORT, 0xE0);
    IOT_OUTP(IOT_PORT, 0x01);
    IOT_OUTP(IOT_PORT, 0x43); // I2C Send指定
    IOT_OUTP(IOT_PORT, 0xC0); // 引数送信指定
    IOT_OUTP(IOT_PORT, len);  // 引数文字数
    for (int i = 0; i < len; i++)
    {
        IOT_OUTP(IOT_PORT, *value); // I2C Send Data
        value++;
    }
    IOT_OUTP(IOT_PORT, 0x00); // End of data
    r = IOT_INP(IOT_PORT);
    return r;
}

//////////////////////////////////////////////////////s
// LCDの制御関数
// LCDの関数はI2CLiquidCrystalをベースにしています。
//////////////////////////////////////////////////////

void lcdWrite(unsigned char dat)
{
    // LCDへの文字データ書き込み関数：LCDへ表示データ書き込みます。
    unsigned char rawdata[2];

    rawdata[0] = 0x40;
    rawdata[1] = dat;
    iotputs_n(I2CADDR_LCD, rawdata, 2);
}
void lcdCommand(unsigned char dat)
{
    // LCDへのコマンド書き込み関数：LCDへ制御データを書き込みます。
    unsigned char rawdata[2];

    rawdata[0] = 0x00;
    rawdata[1] = dat;
    iotputs_n(I2CADDR_LCD, rawdata, 2);
}

void lcdClear(void)
{
    // LCD表示クリア関数：LCDの表示をクリア
    lcdCommand(0x01);
    wait(1); //>1ms wait
}

void lcdsSetCursor(unsigned char col, unsigned char row)
{
    // LCD表示位置設定関数：LCDのカーソルを設定値まで移動します。
    static const unsigned char row_offsets[2] = {0x00, 0x40};
    if (row >= 2)
    {
        row = 2 - 1;
    }
    lcdCommand(0x80 | (row_offsets[row] + col));
}

void lcdBegin(void)
{
    // LCD初期化関数：LCDを使う為の初期化コマンドを送信します。
    // 使う液晶に合わせて設定を変更してみてください。

    // 2進数表現で、001L NF00 形式のデータを送信する。
    // L : データ長。1 = 8ビット, 0 = 4ビット。AQM1602XAは8bitなので1。
    // N : Line数。1 = 2行。0 = 1行。AQM1602XAは2行なので1。
    // F : フォントタイプ。1 = 5x10ドット, 0 = 5x8ドット。AQM1602XAは5x8ドットなので0。
    lcdCommand(0x38);

    // 拡張コマンドモードに移行（内部発振周波数、コントラスト等を設定するため）
    // Function Setで送ったデータの1ビット目を立てたデータを送る
    lcdCommand(0x39);

    // 内部発振周波数
    // 2進数表現で、0001 [BS][F2][F1][F0] 形式のデータを送信する。
    lcdCommand(0x14);

    // コントラスト（1/2）
    // 2進数表現で、0111 [C3][C2][C1][C0] 形式のデータを送信する。
    // データシートより、「5Vの場合、C5=1,C4=0,C3=0,C2=0,C1=1,C0=1で少し濃い目ぐらい」
    lcdCommand(0x73);

    // コントラスト（2/2）
    // 2進数表現で、0101 [Ion][Bon][C5][C4] 形式のデータを送信する。
    // Ion : ICON display on/off。1 = on, 0 = off。Offとしたいので0。
    // Bon : Booster circuit on/off。5Vの場合はBooster回路をOFFとするので0。
    // コントラスト（1/2）より「C5=1,C4=0,C3=0,C2=0,C1=0,C0=0」。
    lcdCommand(0x52);

    // Follower control
    // 2進数表現で、0110 [Fon][Rab2][Rab1][Rab0] 形式のデータを送信する。
    lcdCommand(0x6C);

    // 200msec : for power stable
    wait(200 / 16);

    // 拡張コマンドモードの終了
    // Function Setで送ったデータの1ビット目を倒したデータを送る
    lcdCommand(0x38);

    // Display ON/OFF Control
    // 2進数表現で、0000 1DCB 形式のデータを送信する。
    // D : Display On/Off。1 = On, 0 = Off。Display On なので1。
    // C : Cursor On/Off。1 = On, 0 = Off。Cursor Offとしたいので0。
    // B : Blinking of cursor On/Off。1 = On, 0 = Off。Blinking Offとしたいので0。
    lcdCommand(0x0C);

    // Entry Mode Set
    // 2進数表現で、0000 01DS 形式のデータを送信する。
    // D : Cursor move direction。1 = Increment, 0 = Decrement。カーソル移動方向は増加方向なので1。
    // S : Shift the display。1 = Shiftする, 0 = Shiftしない。Shiftしないとしたいので0。
    lcdCommand(0x06);

    lcdClear(); // Clear display
}
void lcdStr(char *c)
{
    // LCDへの文字列表示関数：カーソル位置から文字列を表示します。
    unsigned char i;
    unsigned char *stringPointer;
    stringPointer = (unsigned char *)c;

    for (i = 0; i < 255; i++) // 桁あふれ防止
    {
        if (*stringPointer == 0x00) // 文字列終了
            return;
        lcdWrite(*stringPointer); // 文字データ書き込み
        stringPointer++;
    }
}

void argv2str(char *argv, unsigned char *strbuffer, int n)
{
    // argvの型変換関数：argvの引数を文字列型に変換する
    unsigned char i;

    for (i = 0; i < n; i++) // 桁あふれ防止
    {
        if (*argv == 0x0d) // 文字列終了
        {
            *strbuffer = 0x00;
            return;
        }
        *strbuffer = *argv;
        strbuffer++;
        argv++;
    }
}
int main(int argc, char *argv[])
{
    unsigned char stringBuffer[128];
    char findDevice;

    int i;

    findDevice = 0;
    printf("\nAQM0802A LCD Sample ver1.00\n");
    printf("Copyrigth 12/01/23 @v9938\n\n");

    if (argc < 2)
    {
        printf("Error:Invalid argument\n");
        return -1;
    }
    int num = iotfindi_("device/i2c_a");
    if (num == 0)
    {
        printf("Error: Device Not Found!");
        return -1;
    }

    char **str = malloc(sizeof(char *) * num);
    for (int i = 0; i < num; i++)
    {
        str[i] = malloc(sizeof(char) * 32);
    }
    int ret = iotfinds_("device/i2c_a", str, num);
    for (i = 0; i < num; i++)
    {
        if (strcmp(str[i], "3E") == 0)
            findDevice = 1;
        free(str[i]);
    }
    free(str);

    if (findDevice == 0)
    {
        printf("Error: LCD not find...\n");
        return 1;
    }

    argv2str(argv[1], stringBuffer, 128);
    printf("LCD Data: %s\n", stringBuffer);
    lcdBegin();
    lcdStr(stringBuffer);

    return 0;
}
