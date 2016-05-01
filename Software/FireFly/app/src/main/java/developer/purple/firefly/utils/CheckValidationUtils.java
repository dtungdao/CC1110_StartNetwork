package developer.purple.firefly.utils;

import android.content.Context;
import android.widget.Toast;

import developer.purple.firefly.common.Constant;

/**
 * Created by tnluan on 4/16/16.
 */
public class CheckValidationUtils {
    public static boolean isNodeAddrValid(Context context, String nodeAddr) {
        if (nodeAddr == null || nodeAddr.length() != 2) {
            Toast.makeText(context, "Node address must have 2 characters", Toast.LENGTH_SHORT).show();
            return false;
        }
        if (!Constant.getHexadecimal().contains(nodeAddr)) {
            Toast.makeText(context, "Each character must be a hexadecimal", Toast.LENGTH_SHORT).show();
            return false;
        }
        return true;
    }
}
