package xt.sample;

import java.util.EnumSet;
import xt.audio.Enums.XtEnumFlags;
import xt.audio.Enums.XtSetup;
import xt.audio.Enums.XtSystem;
import xt.audio.Structs.XtLocation;
import xt.audio.Structs.XtMix;
import xt.audio.Structs.XtVersion;
import xt.audio.XtAudio;
import xt.audio.XtDevice;
import xt.audio.XtDeviceList;
import xt.audio.XtException;
import xt.audio.XtPlatform;
import xt.audio.XtService;
import java.util.Optional;

public class PrintDetailed {

    static void onError(XtLocation location, String message) {
        System.out.println(location + ": " + message);
    }

    static void PrintDevices(XtService service, XtDeviceList list) {
        for(int d = 0; d < list.getCount(); d++) {
            String id = list.getId(d);
            try(XtDevice device = service.openDevice(id)) {
                Optional<XtMix> mix = device.getMix();
                System.out.println("    Device " + id + ":");
                System.out.println("      Name: " + list.getName(id));
                System.out.println("      Capabilities: " + list.getCapabilities(id));
                System.out.println("      Input channels: " + device.getChannelCount(false));
                System.out.println("      Output channels: " + device.getChannelCount(true));
                System.out.println("      Interleaved access: " + device.supportsAccess(true));
                System.out.println("      Non-interleaved access: " + device.supportsAccess(false));
                if(mix.isPresent())
                    System.out.println("      Current mix: " + mix.get().rate + " " + mix.get().sample);
            } catch(XtException e) {
                System.out.println(XtAudio.getErrorInfo(e.getError()));
            }
        }
    }

    public static void main() throws Exception {
        try(XtPlatform platform = XtAudio.init("Sample", null, PrintDetailed::onError)) {
            XtVersion version = XtAudio.getVersion();
            System.out.println("Version: " + version.major + "." + version.minor);
            XtSystem pro = platform.setupToSystem(XtSetup.PRO_AUDIO);
            System.out.println("Pro Audio: " + pro + " (" + (platform.getService(pro) != null) + ")");
            XtSystem system = platform.setupToSystem(XtSetup.SYSTEM_AUDIO);
            System.out.println("System Audio: " + system + " (" + (platform.getService(system) != null) + ")");
            XtSystem consumer = platform.setupToSystem(XtSetup.CONSUMER_AUDIO);
            System.out.println("Consumer Audio: " + consumer + " (" + (platform.getService(consumer) != null) + ")");

            for(XtSystem s: platform.getSystems()) {
                XtService service = platform.getService(s);
                System.out.println("System " + s + ":");
                System.out.println("  Capabilities: " + service.getCapabilities());
                try(XtDeviceList all = service.openDeviceList(EnumSet.of(XtEnumFlags.ALL))) {
                    String defaultInputId = service.getDefaultDeviceId(false);
                    if(defaultInputId != null) {
                        String name = all.getName(defaultInputId);
                        System.out.println("  Default input: " + name + " (" + defaultInputId + ")");
                    }
                    String defaultOutputId = service.getDefaultDeviceId(true);
                    if(defaultOutputId != null) {
                        String name = all.getName(defaultOutputId);
                        System.out.println("  Default output: " + name + " (" + defaultOutputId + ")");
                    }
                }
                try(XtDeviceList inputs = service.openDeviceList(EnumSet.of(XtEnumFlags.INPUT))) {
                    System.out.println("  Input device count: " + inputs.getCount());
                    PrintDevices(service, inputs);
                }
                try(XtDeviceList outputs = service.openDeviceList(EnumSet.of(XtEnumFlags.OUTPUT))) {
                    System.out.println("  Output device count: " + outputs.getCount());
                    PrintDevices(service, outputs);
                }
            }
        } catch(XtException e) {
            System.out.println(XtAudio.getErrorInfo(e.getError()));
        }
    }
}