//
// filter_resamp2_crcf_interp.c
//

#include <stdio.h>
#include <string.h>
#include <complex.h>
#include <math.h>

#include "liquid.h"
#include "liquid.doc.h"

#define OUTPUT_FILENAME_TIME "figures.gen/filter_resamp2_crcf_interp_time.gnu"
#define OUTPUT_FILENAME_FREQ "figures.gen/filter_resamp2_crcf_interp_freq.gnu"

int main() {
    unsigned int m=5;           // filter semi-length (h_len = 4*m+1)
    unsigned int n=100;         // number of input samples
    float As=60.0f;             // stop-band attenuation [dB]

    unsigned int i;

    // derived values: number of input, output samples (adjusted for filter delay)
    unsigned int nx = n + 2*m+1;
    unsigned int ny = 2*nx;

    // allocate memory for buffers
    float complex x[nx];
    float complex y[ny];

    // generate input signal (filtered noise)
    int ftype           = LIQUID_IIRDES_ELLIP;
    int btype           = LIQUID_IIRDES_LOWPASS;
    int format          = LIQUID_IIRDES_SOS;
    unsigned int order  = 7;
    float fc            = 0.17f;
    float f0            = 0.0f;
    float ap            = 1.0f;
    float as            = 80.0f;
    iirfilt_crcf f = iirfilt_crcf_create_prototype(ftype, btype, format, order, fc, f0, ap, as);
    for (i=0; i<nx; i++) {
        float complex z = (i < n) ? randnf() + _Complex_I*randnf() : 0.0f;
        iirfilt_crcf_execute(f, z, &x[i]);
        //x[i] *= (i < n) ? hamming(i,n) : 0.0f;
    }
    iirfilt_crcf_destroy(f);

    // create resampler
    resamp2_crcf q = resamp2_crcf_create(m,0.0f,As);

    // interpolate
    for (i=0; i<nx; i++) {
        // execute resampler, storing in output buffer
        resamp2_crcf_interp_execute(q, x[i], &y[2*i]);
    }

    // clean up allocated objects
    resamp2_crcf_destroy(q);


    // 
    // export output files
    //
    FILE * fid;

    // 
    // export time plot
    //
    fid = fopen(OUTPUT_FILENAME_TIME,"w");
    fprintf(fid,"# %s: auto-generated file\n\n", OUTPUT_FILENAME_TIME);
    fprintf(fid,"reset\n");
    // TODO : switch terminal types here
    fprintf(fid,"set terminal postscript eps enhanced color solid rounded\n");
    //fprintf(fid,"set xrange [0:%u];\n",n);
    fprintf(fid,"set yrange [-1.5:1.5]\n");
    fprintf(fid,"set size ratio 0.3\n");
    fprintf(fid,"set xlabel 'Input Sample Index'\n");
    fprintf(fid,"set key top right nobox\n");
    fprintf(fid,"set ytics -5,1,5\n");
    fprintf(fid,"set grid xtics ytics\n");
    fprintf(fid,"set pointsize 0.6\n");
    fprintf(fid,"set grid linetype 1 linecolor rgb '%s' lw 1\n", LIQUID_DOC_COLOR_GRID);
    fprintf(fid,"set multiplot layout 2,1 scale 1.0,1.0\n");

    fprintf(fid,"# real\n");
    fprintf(fid,"set ylabel 'Real'\n");
    fprintf(fid,"plot '-' using 1:2 with linespoints pointtype 7 linetype 1 linewidth 1 linecolor rgb '#999999' title 'original',\\\n");
    fprintf(fid,"     '-' using 1:2 with points pointtype 7 linecolor rgb '#008000' title 'interpolated'\n");
    // export input signal
    for (i=0; i<nx; i++)
        fprintf(fid,"%12.4e %12.4e %12.4e\n", (float)i, crealf(x[i]), cimagf(x[i]));
    fprintf(fid,"e\n");

    // export output signal
    for (i=0; i<ny; i++)
        fprintf(fid,"%12.4e %12.4e %12.4e\n", 0.5f*(float)i - (float)m, crealf(y[i]), cimagf(y[i]));
    fprintf(fid,"e\n");

    fprintf(fid,"# imag\n");
    fprintf(fid,"set ylabel 'Imag'\n");
    fprintf(fid,"plot '-' using 1:3 with linespoints pointtype 7 linetype 1 linewidth 1 linecolor rgb '#999999' title 'original',\\\n");
    fprintf(fid,"     '-' using 1:3 with points pointtype 7 linecolor rgb '#800000' title 'interpolated'\n");

    // export input signal
    for (i=0; i<nx; i++)
        fprintf(fid,"%12.4e %12.4e %12.4e\n", (float)i, crealf(x[i]), cimagf(x[i]));
    fprintf(fid,"e\n");

    // export output signal
    for (i=0; i<ny; i++)
        fprintf(fid,"%12.4e %12.4e %12.4e\n", 0.5f*(float)i - (float)m, crealf(y[i]), cimagf(y[i]));
    fprintf(fid,"e\n");

    fprintf(fid,"unset multiplot\n");

    // close output file
    fclose(fid);


    // 
    // export spectrum plot
    //
    fid = fopen(OUTPUT_FILENAME_FREQ,"w");
    unsigned int nfft = 512;
    float complex X[nfft];
    float complex Y[nfft];
    liquid_doc_compute_psdcf(x,nx,X,nfft,LIQUID_DOC_PSDWINDOW_HANN,0);
    liquid_doc_compute_psdcf(y,ny,Y,nfft,LIQUID_DOC_PSDWINDOW_HANN,0);
    fft_shift(X,nfft);
    fft_shift(Y,nfft);
    float scaling_factor = 20*log10f(nfft);

    fprintf(fid,"# %s: auto-generated file\n\n", OUTPUT_FILENAME_FREQ);
    fprintf(fid,"reset\n");
    // TODO : switch terminal types here
    fprintf(fid,"set terminal postscript eps enhanced color solid rounded\n");
    fprintf(fid,"set xrange [-0.5:0.5];\n");
    fprintf(fid,"set yrange [-140:0]\n");
    fprintf(fid,"set size ratio 0.6\n");
    fprintf(fid,"set xlabel 'Normalized Output Frequency'\n");
    fprintf(fid,"set ylabel 'Power Spectral Density [dB]'\n");
    fprintf(fid,"set key top right nobox\n");
    fprintf(fid,"set grid xtics ytics\n");
    fprintf(fid,"set pointsize 0.6\n");
    fprintf(fid,"set grid linetype 1 linecolor rgb '%s' lw 1\n",LIQUID_DOC_COLOR_GRID);

    fprintf(fid,"# real\n");
    fprintf(fid,"plot '-' using 1:2 with lines linetype 1 linewidth 4 linecolor rgb '#999999' title 'original',\\\n");
    fprintf(fid,"     '-' using 1:2 with lines linetype 1 linewidth 4 linecolor rgb '#004080' title 'interpolated'\n");
    // export output
    for (i=0; i<nfft; i++) {
        float fx = ((float)(i) / (float)nfft - 0.5f)*0.5f;
        fprintf(fid,"%12.8f %12.4e\n", fx, 20*log10f(cabsf(X[i])) - scaling_factor);
    }
    fprintf(fid,"e\n");
    for (i=0; i<nfft; i++) {
        float fy = ((float)(i) / (float)nfft - 0.5f);
        fprintf(fid,"%12.8f %12.4e\n", fy, 20*log10f(cabsf(Y[i])) - scaling_factor - 20*log10(2.0f));
    }
    fprintf(fid,"e\n");

    fclose(fid);

    printf("done.\n");
    return 0;
}
