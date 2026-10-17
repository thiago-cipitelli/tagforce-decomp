/* Cross-subsystem pieces of BOOT ("modehsys"): input state, frame scheduling. Grows on demand. */
#ifndef EHSYS_H
#define EHSYS_H

typedef struct PadState {
    unsigned int unk0;
    unsigned int held;      /* +0x04 buttons currently down (PSP_CTRL_*) */
    unsigned int unk8[7];
    unsigned int pressed;   /* +0x24 buttons that went down this frame */
} PadState;

PadState *pad_get(void);

void ehscript_wake_main(void);   /* sceKernelWakeupThread(main): hands the frame back to BOOT's main loop */
void sys_frame_wait(void);       /* called by scenes once per frame */

#endif
