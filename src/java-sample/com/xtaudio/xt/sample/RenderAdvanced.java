package com.xtaudio.xt.sample;

import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.xtaudio.xt.XtAudio;
import com.xtaudio.xt.XtDevice;
import com.xtaudio.xt.XtService;
import com.xtaudio.xt.XtStream;
import static com.xtaudio.xt.NativeTypes.*;

public class RenderAdvanced {

    static double phase = 0.0;
    static final double FREQUENCY = 440.0;

    static void readLine() {
        System.out.println("Press any key to continue...");
        System.console().readLine();
    }

    static float nextSine(double sampleRate) {
        phase += FREQUENCY / sampleRate;
        if (phase >= 1.0)
            phase = -1.0;
        return (float) Math.sin(2.0 * phase * Math.PI);
    }

    static void xRun(int index, Object user) {
        // Don't do this.
        System.out.println("XRun on device " + index + ", user = " + user + ".");
    }

    static void renderInterleaved(XtStream stream, Object input, Object output, int frames,
            double time, long position, boolean timeValid, long error, Object user) throws Exception {

        XtFormat format = stream.getFormat();
        for (int f = 0; f < frames; f++) {
            float sine = nextSine(format.mix.rate);
            for (int c = 0; c < format.channels.outputs; c++)
                ((float[]) output)[f * format.channels.outputs + c] = sine;
        }
    }

    static void renderInterleavedRaw(XtStream stream, Object input, Object output, int frames,
            double time, long position, boolean timeValid, long error, Object user) throws Exception {

        XtFormat format = stream.getFormat();
        int sampleSize = XtAudio.getSampleAttributes(format.mix.sample).size;
        for (int f = 0; f < frames; f++) {
            float sine = nextSine(format.mix.rate);
            for (int c = 0; c < format.channels.outputs; c++)
                ((Pointer) output).setFloat((f * format.channels.outputs + c) * sampleSize, sine);
        }
    }

    static void renderNonInterleaved(XtStream stream, Object input, Object output, int frames,
            double time, long position, boolean timeValid, long error, Object user) throws Exception {

        XtFormat format = stream.getFormat();
        for (int f = 0; f < frames; f++) {
            float sine = nextSine(format.mix.rate);
            for (int c = 0; c < format.channels.outputs; c++)
                ((float[][]) output)[c][f] = sine;
        }
    }

    static void renderNonInterleavedRaw(XtStream stream, Object input, Object output, int frames,
            double time, long position, boolean timeValid, long error, Object user) throws Exception {

        XtFormat format = stream.getFormat();
        int sampleSize = XtAudio.getSampleAttributes(format.mix.sample).size;
        for (int f = 0; f < frames; f++) {
            float sine = nextSine(format.mix.rate);
            for (int c = 0; c < format.channels.outputs; c++)
                ((Pointer) output).getPointer(c * Native.POINTER_SIZE).setFloat(f * sampleSize, sine);
        }
    }

    public static void main(String[] args) throws Exception {

        try (XtAudio audio = new XtAudio(null, null, null)) {

            var system = XtAudio.setupToSystem(XtSetup.CONSUMER_AUDIO);
            XtService service = XtAudio.getService(system);
            if (service == null)
                return;

            XtFormat format = new XtFormat(new XtMix(44100, XtSample.FLOAT32), new XtChannels(0, 0, 2, 0));
            try (XtDevice device = service.openDefaultDevice(true)) {
                if (device == null || !device.supportsFormat(format))
                    return;

                XtBufferSize size = device.getBufferSize(format);

                try (XtStream stream = device.openStream(format, true, false, size.current,
                        RenderAdvanced::renderInterleaved, RenderAdvanced::xRun, "user-data")) {
                    stream.start();
                    System.out.println("Rendering interleaved...");
                    readLine();
                    stream.stop();
                }

                try (XtStream stream = device.openStream(format, true, true, size.current,
                        RenderAdvanced::renderInterleavedRaw, RenderAdvanced::xRun, "user-data")) {
                    stream.start();
                    System.out.println("Rendering interleaved, raw buffers...");
                    readLine();
                    stream.stop();
                }

                try (XtStream stream = device.openStream(format, false, false, size.current,
                        RenderAdvanced::renderNonInterleaved, RenderAdvanced::xRun, "user-data")) {
                    stream.start();
                    System.out.println("Rendering non-interleaved...");
                    readLine();
                    stream.stop();
                }

                try (XtStream stream = device.openStream(format, false, true, size.current,
                        RenderAdvanced::renderNonInterleavedRaw, RenderAdvanced::xRun, "user-data")) {
                    stream.start();
                    System.out.println("Rendering non-interleaved, raw buffers...");
                    readLine();
                    stream.stop();
                }

                XtFormat sendTo0 = new XtFormat(new XtMix(44100, XtSample.FLOAT32), new XtChannels(0, 0, 1, 1L << 0));
                try (XtStream stream = device.openStream(sendTo0, true, false, size.current,
                        RenderAdvanced::renderInterleaved, RenderAdvanced::xRun, "user-data")) {
                    stream.start();
                    System.out.println("Rendering channel mask, channel 0...");
                    readLine();
                    stream.stop();
                }

                XtFormat sendTo1 = new XtFormat(new XtMix(44100, XtSample.FLOAT32), new XtChannels(0, 0, 1, 1L << 1));
                try (XtStream stream = device.openStream(sendTo1, true, false, size.current,
                        RenderAdvanced::renderInterleaved, RenderAdvanced::xRun, "user-data")) {
                    stream.start();
                    System.out.println("Rendering channel mask, channel 1...");
                    readLine();
                    stream.stop();
                }
            }
        }
    }
}
