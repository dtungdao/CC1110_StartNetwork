package developer.purple.firefly.common;

import java.util.ArrayList;

/**
 * Created by luantran on 4/5/16.
 */
public class Constant {
    private static ArrayList<String> mHexadecimal;

    public static ArrayList<String> getHexadecimal() {
        if (mHexadecimal == null) {
            mHexadecimal = new ArrayList<>();
            String[] validChar = new String[] {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F"};
            for (int i = 0; i < validChar.length; i++) {
                for (int j = 0; j < validChar.length; j++) {
                    mHexadecimal.add(validChar[i] + validChar[j]);
                }
            }
        }
        return mHexadecimal;
    }
}
