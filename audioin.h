#ifndef AUDIOIN_H
#define AUDIOIN_H
#include <portaudio.h>
#include <cstring>
#include <QDebug>
#include <fftw++.h>
#include <Array.h>
#include "add_functions.h"
#define SAMPLE_RATE  (44100)
#define FRAMES_PER_BUFFER (512)
#define NUM_CHANNELS    (1)
/* #define DITHER_FLAG     (paDitherOff) */
#define DITHER_FLAG     (0) /**/
/** Set to 1 if you want to capture the recording to a file. */
#define WRITE_TO_FILE   (0)

#define FORMAT 2


/* Select sample format. */
#if FORMAT == 2
#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"
#elif FORMAT == 1
#define PA_SAMPLE_TYPE  paInt16
typedef short SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#elif FORMAT == 0
#define PA_SAMPLE_TYPE  paInt8
typedef char SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#else
#define PA_SAMPLE_TYPE  paUInt8
typedef unsigned char SAMPLE;
#define SAMPLE_SILENCE  (128)
#define PRINTF_S_FORMAT "%d"
#endif

typedef struct
{
    int          frameIndex;  /* Index into sample array. */
    int          maxFrameIndex;
    SAMPLE      *recordedSamples;
}
paTestData;

class AudioIn : public QObject
{
    Q_OBJECT
private:
    PaStreamParameters InputParams;
    PaStream*           stream;
    PaError             err = paNoError;
    paTestData          data;
    int                 i;
    int                 totalFrames;
    int                 numSamples;
    int                 numBytes;
    SAMPLE              max, neg_max, val;
    double              average;

    void applyHanningWindow(void);
signals:
    void currentAmp(double val);
    void currentSound(QVector<double> data);
public slots:
    void maxAmplitude(void);
    void getAudioData(void);
    void getFFTdata(void);
public:
    AudioIn();
    ~AudioIn();

    void initAudioIn(const PaStreamParameters &params, int seconds);
    void testAudioIn(void);

};

#endif // AUDIOIN_H






