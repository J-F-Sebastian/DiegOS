#include <platform_config.h>
#include <diegos/kernel.h>
#include <libs/bitmaps.h>

#if defined(ENABLE_SIMD) || defined(ENABLE_FP)

static uint8_t last_context = (uint8_t)-1U;
static long context_valid[BITMAPLEN(DIEGOS_MAX_THREADS)] = {0};

/*
 * ENABLE_SIMD FIRST. SIMD exception handler will handle FP/MMX too, but
 * not the reverse (i.e. FP cannot handle simd exceptions)
 */

#if defined(ENABLE_SIMD)
/*
 * Must be aligned to 16 bytes
 */
static char simd_context[DIEGOS_MAX_THREADS][512] __attribute__((aligned(16)));

#elif defined(ENABLE_FP)
/*
 * There is no real requirement for alignment in case of FP as we are using
 * fsave/frstor but doubles are natually aligned to 8...
 */
static char fp_context[DIEGOS_MAX_THREADS][108] __attribute__((aligned(8)));
#endif

void exc_handler_fp (void)
{
    uint8_t tid = my_thread_id();

    /*
     * We need to clean up the TS bit at all times!
     */
    __asm__ volatile ("clts\n\t":::);
    
    /*
     * First instruction ever - mark the context to be saved
     */
    if ((uint8_t)-1U == last_context) {
        last_context = tid;
    }
    /*
     * Same context running - same running thread is using fp instructions,
     * no interference from other threads.
     * If tid != last_fp_context, then they are different
     */
    else if (tid != last_context) {
        /*
         * Save context
         */
#if defined(ENABLE_SIMD)        
        __asm__ volatile ("fxsave %0\n\t" :: "m" (simd_context[last_context]):);
#elif defined(ENABLE_FP)
        __asm__ volatile ("fsave %0\n\t":: "m" (fp_context[last_context]):);
#endif    
        bitmap_set(context_valid, last_context);
        if (bitmap_is_set(context_valid, tid)) {
            /*
             * Restore context
             */
#if defined(ENABLE_SIMD)            
            __asm__ volatile ("fxrstor %0\n\t":: "m" (simd_context[tid]):);
#elif defined(ENABLE_FP)            
            __asm__ volatile ("frstor %0\n\t":: "m" (fp_context[tid]):);
#endif            
        }
#if defined(ENABLE_SIMD)        
          else {
            __asm__ volatile ("finit\n\t");            
        }
#endif        
        last_context = tid;
    }    
}

#endif

void cleanup_context(void *ctx, unsigned tid)
{
#if defined(ENABLE_SIMD) || defined(ENABLE_FP)
    bitmap_clear(context_valid, tid);
#endif
}
