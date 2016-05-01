package developer.purple.firefly.model;

import java.util.List;

/**
 * Created by tnluan on 4/6/16.
 */
public class SetItem {
    private String mName;
    private List<NodeModel> mNodeModels;

    public SetItem(){

    }

    public SetItem(String mName, List<NodeModel> mNodeModels) {
        this.mName = mName;
        this.mNodeModels = mNodeModels;
    }

    public String getName() {
        return mName;
    }

    public void setName(String mName) {
        this.mName = mName;
    }

    public List<NodeModel> getNodeModels() {
        return mNodeModels;
    }

    public void setNodeModels(List<NodeModel> mNodeModels) {
        this.mNodeModels = mNodeModels;
    }
}
