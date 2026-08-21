#include <stdio.h>
#include <math.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xaxidma.h"
#include "xil_cache.h"
#include "xil_types.h"

// Radar & FFT Configuration
#define NUM_BINS            1024
#define SAMPLING_FREQ_MHZ   40.0f
#define BYTES_PER_SAMPLE    4                       // 16-bit I + 16-bit Q = 4 bytes per bin
#define DMA_BUFFER_SIZE     (NUM_BINS * BYTES_PER_SAMPLE)

// In Vitis Unified / SDT environments, use XPAR_XAXIDMA_0_BASEADDR or standard ID
#if defined(XPAR_AXIDMA_0_DEVICE_ID)
    #define DMA_DEV_ID      XPAR_AXIDMA_0_DEVICE_ID
#else
    #define DMA_DEV_ID      0
#endif

// 64-byte aligned buffer in DDR to match Zynq cache line boundary
static int16_t rx_fft_buffer[NUM_BINS * 2] __attribute__((aligned(64)));

static XAxiDma AxiDma;

void process_and_display_spectrum(const int16_t *buffer, int num_bins) {
    int peak_bin = 0;
    float peak_mag = 0.0f;
    float sum_mag = 0.0f;

    for (int i = 0; i < num_bins; i++) {
        int16_t real = buffer[2 * i];
        int16_t imag = buffer[2 * i + 1];

        // Complex magnitude: |X[k]| = sqrt(Re^2 + Im^2)
        float mag = sqrtf((float)((int32_t)real * real + (int32_t)imag * imag));
        sum_mag += mag;

        if (mag > peak_mag) {
            peak_mag = mag;
            peak_bin = i;
        }
    }

    float avg_noise_floor = sum_mag / (float)num_bins;
    float detected_freq_mhz = ((float)peak_bin * SAMPLING_FREQ_MHZ) / (float)num_bins;

    // Print summary to terminal
    printf("========================================\r\n");
    printf(" Live PL FFT Capture Status\r\n");
    printf("========================================\r\n");
    printf("Peak Bin Index : %d / %d\r\n", peak_bin, num_bins);
    printf("Peak Magnitude : %.2f\r\n", peak_mag);
    printf("Avg Noise Floor: %.2f\r\n", avg_noise_floor);
    printf("Detected Freq  : %.3f MHz\r\n", detected_freq_mhz);
    printf("SNR Estimate   : %.2f dB\r\n", 20.0f * log10f((peak_mag + 1e-6f) / (avg_noise_floor + 1e-6f)));
    printf("\r\n");
}

int init_dma_subsystem(void) {
    XAxiDma_Config *cfg_ptr;
    int status;

    cfg_ptr = XAxiDma_LookupConfig(DMA_DEV_ID);
    if (!cfg_ptr) {
        xil_printf("ERROR: No DMA configuration found for ID %d\r\n", DMA_DEV_ID);
        return XST_FAILURE;
    }

    status = XAxiDma_CfgInitialize(&AxiDma, cfg_ptr);
    if (status != XST_SUCCESS) {
        xil_printf("ERROR: DMA Initialization failed with status %d\r\n", status);
        return XST_FAILURE;
    }

    if (XAxiDma_HasSg(&AxiDma)) {
        xil_printf("ERROR: DMA configured in Scatter-Gather mode. Direct register mode required.\r\n");
        return XST_FAILURE;
    }

    // Disable all S2MM interrupts for polling mode operation
    XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);

    return XST_SUCCESS;
}

int main(void) {
    init_platform();

    printf("\r\n========================================\r\n");
    printf(" Eclypse Z7 FMCW Radar Signal Processor\r\n");
    printf("========================================\r\n");

    if (init_dma_subsystem() != XST_SUCCESS) {
        printf("Halting execution due to DMA failure.\r\n");
        cleanup_platform();
        return -1;
    }

    printf("AXI DMA Initialized in Simple Transfer Mode.\r\n");
    printf("Starting live capture loop from FPGA PL...\r\n\r\n");

    while (1) {
        // 1. Flush CPU cache line covering destination DDR buffer
        Xil_DCacheFlushRange((UINTPTR)rx_fft_buffer, DMA_BUFFER_SIZE);

        // 2. Launch DMA S2MM transfer (PL FFT Stream -> DDR)
        int status = XAxiDma_SimpleTransfer(&AxiDma, (UINTPTR)rx_fft_buffer, DMA_BUFFER_SIZE, XAXIDMA_DEVICE_TO_DMA);
        if (status != XST_SUCCESS) {
            printf("DMA Simple Transfer failed to initiate! Status: %d\r\n", status);
            break;
        }

        // 3. Poll until transfer finishes
        while (XAxiDma_Busy(&AxiDma, XAXIDMA_DEVICE_TO_DMA)) {
            // Wait for 1024-point packet to complete
        }

        // 4. Invalidate cache so ARM core reads updated data written to DDR by DMA
        Xil_DCacheInvalidateRange((UINTPTR)rx_fft_buffer, DMA_BUFFER_SIZE);

        // 5. Compute peak magnitude & analyze spectrum
        process_and_display_spectrum(rx_fft_buffer, NUM_BINS);

        // Frame rate limiter (~500 ms delay)
        for (volatile uint32_t delay = 0; delay < 25000000; delay++);
    }

    cleanup_platform();
    return 0;
}