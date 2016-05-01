package developer.purple.firefly.manager;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

import developer.purple.firefly.model.SetItem;

/**
 * Created by tnluan on 4/14/16.
 */
public class DataManager {
    private static ArrayList<SetItem> setModelList;

    public static ArrayList<SetItem> getSetList() {
        if (setModelList == null) {
            setModelList = new ArrayList<>();
        }
        return setModelList;
    }

    public static SetItem getSet(int position) {
        if (setModelList == null) {
            setModelList = new ArrayList<>();
        }
        if (setModelList.size() > position) {
            return setModelList.get(position);
        }
        return null;
    }

    public static void addNewSet(SetItem setModel) {
        if (setModelList == null) {
            setModelList = new ArrayList<>();
        }
        setModelList.add(setModel);
    }

    public static void deleteSet(SetItem setModel) {
        if (setModelList == null) {
            setModelList = new ArrayList<>();
        }
        setModelList.remove(setModel);
    }
}
