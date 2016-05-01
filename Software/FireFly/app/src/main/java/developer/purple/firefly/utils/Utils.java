package developer.purple.firefly.utils;

import android.content.Context;
import android.os.Environment;
import android.util.Log;
import android.util.TypedValue;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.text.SimpleDateFormat;
import java.util.Calendar;

/**
 * Created by tnluan on 4/6/16.
 */
public class Utils {
    /**
     * Convert dp to px
     *
     * @param context
     * @param dp
     * @return
     */
    public static int convertDpToPx(Context context, int dp) {
        return (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, dp, context.getResources().getDisplayMetrics());
    }

    /**
     * Convert sp to px
     *
     * @param context
     * @param dp
     * @return
     */
    public static int convertSpToPx(Context context, int dp) {
        return (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_SP, dp, context.getResources().getDisplayMetrics());
    }

    /**
     * Get screen width
     *
     * @param context
     * @return
     */
    public static int getScreenWidth(Context context) {
        return context.getResources().getDisplayMetrics().widthPixels;
    }

    /**
     * Get screen height
     *
     * @param context
     * @return
     */
    public static int getScreenHeight(Context context) {
        return context.getResources().getDisplayMetrics().heightPixels;
    }

    public static void writeToFile(String data) {
        Calendar c = Calendar.getInstance();
        SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS");
        String formattedDate = df.format(c.getTime());
        data = formattedDate + " " + data;

        String oldData = readFromFile();
        data = oldData + data;

        String filePath = Environment.getExternalStorageDirectory() + "/FireflyLog.txt";
        try {
            FileOutputStream fileOutputStream = new FileOutputStream(filePath);
            OutputStreamWriter outputStreamWriter = new OutputStreamWriter(fileOutputStream);
            outputStreamWriter.write(data);
            outputStreamWriter.close();
        } catch (Exception e) {
            Log.e("Exception", "File write failed: " + e.toString());
        }
    }

    public static String readFromFile() {
        String ret = "";
        String filePath = Environment.getExternalStorageDirectory() + "/FireflyLog.txt";
        try {
            FileInputStream fileInputStream = new FileInputStream(filePath);

            if (fileInputStream != null) {
                InputStreamReader inputStreamReader = new InputStreamReader(fileInputStream);
                BufferedReader bufferedReader = new BufferedReader(inputStreamReader);
                String receiveString = "";
                StringBuilder stringBuilder = new StringBuilder();

                while ((receiveString = bufferedReader.readLine()) != null) {
                    stringBuilder.append(receiveString);
                    stringBuilder.append("\n");
                }

                fileInputStream.close();
                ret = stringBuilder.toString();
            }
        } catch (Exception e) {
            Log.e("Exception", "Can not read file: " + e.toString());
        }

        return ret;
    }
}
