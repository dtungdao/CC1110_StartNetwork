package developer.purple.firefly.adapter;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

import java.util.List;

import developer.purple.firefly.R;
import developer.purple.firefly.model.NodeModel;
import developer.purple.firefly.model.SetItem;

/**
 * Created by dkan10 on 4/14/16.
 */
public class NewNodeAdapter extends ArrayAdapter<NodeModel> {

    Context context;
    int resource;
    List<NodeModel> nodeModels;

    public NewNodeAdapter(Context context, int resource, List<NodeModel> objects) {
        super(context, resource, objects);
        this.context = context;
        this.resource = resource;
        this.nodeModels = objects;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        ViewHolder holder = null;
        if (convertView == null) {
            LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflater.inflate(resource, null);
            holder = new ViewHolder(convertView);
            convertView.setTag(holder);
        } else {
            holder = (ViewHolder) convertView.getTag();
        }
        holder.setInfo(nodeModels.get(position));
        return convertView;
    }

    public class ViewHolder {
        TextView tvNodeName;

        public ViewHolder(View rootView) {
            tvNodeName = (TextView) rootView.findViewById(R.id.tvNodeName);
        }

        public void setInfo(NodeModel nodeModel) {
            tvNodeName.setText(nodeModel.getNodeAddr());
        }
    }
}
