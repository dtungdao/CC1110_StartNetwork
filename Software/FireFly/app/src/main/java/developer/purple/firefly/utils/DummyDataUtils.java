package developer.purple.firefly.utils;

import android.graphics.Point;

import java.util.ArrayList;
import java.util.List;

import developer.purple.firefly.model.NodeModel;
import developer.purple.firefly.model.SetItem;

/**
 * Created by tnluan on 4/6/16.
 */
public class DummyDataUtils {
    public static SetItem getDummyUser() {
        SetItem userModel = new SetItem();
        userModel.setName("dkan");

        List<NodeModel> nodeModels = new ArrayList<>();
        Point point = new Point();
        point.set(0, 0);
        NodeModel nodeModel = new NodeModel("00", false, point);
        nodeModels.add(nodeModel);

        point = new Point();
        point.set(120, 100);
        nodeModel = new NodeModel("11", false, point);
        nodeModels.add(nodeModel);

        point = new Point();
        point.set(550, 250);
        nodeModel = new NodeModel("A7", false, point);
        nodeModels.add(nodeModel);

        point = new Point();
        point.set(220, 800);
        nodeModel = new NodeModel("DD", false, point);
        nodeModels.add(nodeModel);

        userModel.setNodeModels(nodeModels);
        return userModel;
    }
}
