#include "audioin.h"
//Helper functions
/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int recordCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    paTestData *data = (paTestData*)userData;
    const SAMPLE *rptr = (const SAMPLE*)inputBuffer;
    SAMPLE *wptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
    long framesToCalc;
    long i;
    int finished;
    unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;

    (void) outputBuffer; /* Prevent unused variable warnings. */
    (void) timeInfo;
    (void) statusFlags;
    (void) userData;

    if( framesLeft < framesPerBuffer )
    {
        framesToCalc = framesLeft;
        finished = paComplete;
    }
    else
    {
        framesToCalc = framesPerBuffer;
        finished = paContinue;
    }

    if( inputBuffer == nullptr )
    {
        for( i=0; i<framesToCalc; i++ )
        {
            *wptr++ = SAMPLE_SILENCE;  /* left */
            if( NUM_CHANNELS == 2 ) *wptr++ = SAMPLE_SILENCE;  /* right */
        }
    }
    else
    {
        for( i=0; i<framesToCalc; i++ )
        {
            *wptr++ = *rptr++;  /* left */
            if( NUM_CHANNELS == 2 ) *wptr++ = *rptr++;  /* right */
        }
    }
    data->frameIndex += framesToCalc;
    return finished;
}

AudioIn::AudioIn()
{

}

AudioIn::~AudioIn()
{
    if(this->data.recordedSamples != nullptr)
        delete[] this->data.recordedSamples;
}

void AudioIn::initAudioIn(const PaStreamParameters &params, int seconds)
{
    this->InputParams = params;
    this->data.maxFrameIndex = seconds*SAMPLE_RATE;
    this->data.frameIndex = 0;
    this->numSamples = seconds * SAMPLE_RATE * NUM_CHANNELS;
    this->numBytes = numSamples * sizeof(SAMPLE);
    this->data.recordedSamples = new SAMPLE[this->numBytes];
    if(this->data.recordedSamples == nullptr)
        return;
    memset(this->data.recordedSamples, 0, this->numBytes);
    this->err = Pa_Initialize();
    if(err != paNoError)
        return;
    this->InputParams.device = Pa_GetDefaultInputDevice(); /* default input device */
    if (this->InputParams.device == paNoDevice) {
        return;
    }
    this->InputParams.channelCount = 1;                    /* mono input */
    this->InputParams.sampleFormat = PA_SAMPLE_TYPE;
    this->InputParams.suggestedLatency = Pa_GetDeviceInfo(this->InputParams.device)->defaultLowInputLatency;
    this->InputParams.hostApiSpecificStreamInfo = NULL;
}

void AudioIn::testAudioIn()
{
    this->err = Pa_OpenStream(
                  &(this->stream),
                  &(this->InputParams),
                  NULL,                  /* &outputParameters, */
                  SAMPLE_RATE,
                  FRAMES_PER_BUFFER,
                  paClipOff,      /* we won't output out of range samples so don't bother clipping them */
                  recordCallback,
                  &data );
    if( this->err != paNoError )
    {
        //qDebug() << "Error: " << Pa_GetErrorText(this->err);
        debug_out("Error: " + QString(Pa_GetErrorText(this->err)));
        return;
    }
    this->err = Pa_CloseStream(this->stream);
}

void AudioIn::maxAmplitude()
{
    if(this->data.frameIndex == 0)
        this->getAudioData();
    /* Measure maximum peak amplitude. */
    this->max = 0;
    this->neg_max = 0;
    this->average = 0.0;
    for(int i=0; i<numSamples; i++ )
    {
        val = data.recordedSamples[i];
        if( val < 0 )
        {
            if(val <= neg_max)
                neg_max = val;
            average -= val;
        }
        else
        {
            if(val >= max)
                max = val;
            average += val;
        }
    }

    average = average / (double)numSamples;

    //qDebug() << "sample max amplitude = " << QString::number(max) << "\n";
    //qDebug() << "sample average = " << QString::number(average) << "\n";
    debug_out("Sample max amplitude = " + QString::number(max));
    debug_out("Sample average = " + QString::number(average));
    //Clean up
    if(this->data.recordedSamples != NULL)
        memset(this->data.recordedSamples, 0, this->numBytes);
    this->data.frameIndex = 0;
    //Cleaning finished?
    emit this->currentAmp(average);
}

void AudioIn::applyHanningWindow()
{
    for(int i = 0; i < numSamples; i++)
    {
        this->data.recordedSamples[i]*=(0.54-0.46*cos((2*M_PI*i)/((double)(numSamples-1))));
    }
}

void AudioIn::getAudioData()
{
    qDebug() << "Recording now!";
    qDebug() << "Data index 0: " << this->data.frameIndex;
    this->err = Pa_OpenStream(
                  &(this->stream),
                  &(this->InputParams),
                  NULL,                  /* &outputParameters, */
                  SAMPLE_RATE,
                  FRAMES_PER_BUFFER,
                  paClipOff,      /* we won't output out of range samples so don't bother clipping them */
                  recordCallback,
                  &(this->data) );
    this->err = Pa_StartStream( this->stream );
    if( this->err != paNoError )
        return;
    while( ( err = Pa_IsStreamActive( this->stream ) ) == 1 )
    {
        Pa_Sleep(100);
        //qDebug() << "Data index: " << this->data.frameIndex;
    }
    if( this->err < 0 )
    {
        qDebug() << "Error: " << Pa_GetErrorText(this->err);
        return;
    }

    this->err = Pa_CloseStream( stream );
    if( this->err != paNoError )
        return;
}

void AudioIn::getFFTdata()
{
    if(this->data.frameIndex == 0)
        this->getAudioData();
    fftwpp::fftw::maxthreads=get_max_threads();
    unsigned int n=pow(2,17);
    unsigned int np=n/2+1;
    size_t align=sizeof(Complex);
    qDebug() << "Now aquiring the FFTW, building up the arrays for " << numSamples << " data!";
    Array::array1<Complex> F(np,align);
    Array::array1<double> f(n,align);               // For out-of-place transforms
    //  array1<double> f(2*np,(double *) F()); // For in-place transforms

    fftwpp::rcfft1d Forward(n,f,F);
    fftwpp::crfft1d Backward(n,F,f);
    qDebug() << "Applying filtering";
    this->applyHanningWindow();

    QVector<double> Amplitude;

    qDebug() << "Putting " << numSamples << " into an array!";
    for(unsigned int i = 0; i < n; i++)
        f[i] = this->data.recordedSamples[i];

    qDebug() << "input:\n" << f[0] << '\n';

    Forward.fft(f,F);

    qDebug() << "output:\n" << F[0].real() << ", " << F[0].imag() << '\n';

    for(unsigned int i = 0; i < np; i++)
        Amplitude.append(F[i].real()*F[i].real()+F[i].imag()*F[i].imag());
    emit this->currentSound(Amplitude);
    if(this->data.recordedSamples != NULL)
        memset(this->data.recordedSamples, 0, this->numBytes);
    this->data.frameIndex = 0;
}
