package xt.audio;

import com.sun.jna.DefaultTypeMapper;
import com.sun.jna.FromNativeContext;
import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.NativeLibrary;
import com.sun.jna.ToNativeContext;
import com.sun.jna.TypeConverter;
import java.io.File;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.nio.file.Path;
import xt.audio.Enums.XtCause;
import xt.audio.Enums.XtSample;
import xt.audio.Enums.XtSetup;
import xt.audio.Enums.XtSystem;
import xt.audio.Structs.XtErrorInfo;
import java.util.HashMap;
import java.util.Map;
import xt.audio.Structs.XtLocation;

class XtTypeMapper extends DefaultTypeMapper {
    XtTypeMapper() {
        addTypeConverter(XtSetup.class, new EnumConverter<>(XtSetup.class, 0));
        addTypeConverter(XtCause.class, new EnumConverter<>(XtCause.class, 0));
        addTypeConverter(XtSample.class, new EnumConverter<>(XtSample.class, 0));
        addTypeConverter(XtSystem.class, new EnumConverter<>(XtSystem.class, 1));
    }
}

class EnumConverter<E extends Enum<E>> implements TypeConverter {
    final int _base;
    final Class<E> _type;
    EnumConverter(Class<E> type, int base) { _base = base; _type = type; }
    @Override public Class<Integer> nativeType() { return Integer.class; }
    @Override public Object toNative(Object o, ToNativeContext tnc) { return o == null? 0: ((Enum<E>)o).ordinal() + _base; }
    @Override public Object fromNative(Object o, FromNativeContext fnc) { return _type.getEnumConstants()[((int)o) - _base]; }
}

class Utility {
    static final NativeLibrary LIBRARY;
    static {
        URI location = null;
        try {
            location = Utility.class.getProtectionDomain().getCodeSource().getLocation().toURI();
        } catch(URISyntaxException e) {
            throw new RuntimeException(e);
        }
        var folder = new File(location).getParent();
        String prefix = Native.POINTER_SIZE == 8? "x64": "x86";
        var path = Path.of(folder, prefix, System.mapLibraryName("xt-core"));
        System.load(path.toAbsolutePath().toString());
        System.setProperty("jna.encoding", "UTF-8");
        Map<String, Object> options = new HashMap<>();
        options.put(Library.OPTION_TYPE_MAPPER, new XtTypeMapper());
        LIBRARY = NativeLibrary.getInstance("xt-core", options);
        Native.register(LIBRARY);
    }

    static native String XtPrintErrorInfo(XtErrorInfo info);
    static native String XtPrintLocation(XtLocation location);
    static void handleError(long error) { if(error != 0) throw new XtException(error); }
    static <T> T handleError(long error, T result) { if(error != 0) throw new XtException(error); return result; }
}