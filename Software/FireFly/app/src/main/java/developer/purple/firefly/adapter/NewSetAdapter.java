package developer.purple.firefly.adapter;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.List;

import developer.purple.firefly.R;
import developer.purple.firefly.model.SetItem;

/**
 * Created by dkan10 on 4/11/16.
 */
public class NewSetAdapter extends ArrayAdapter<SetItem> {

    Context context;
    int resource;
    List<SetItem> setItem;

    public NewSetAdapter(Context context, int resource, ArrayList<SetItem> objects) {
        super(context, resource, objects);
        this.context = context;
        this.resource = resource;
        this.setItem = objects;
    }

    @Override
    public View getView(final int position, View convertView, final ViewGroup parent) {
        ViewHolder holder = null;
        if (convertView == null) {
            LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflater.inflate(resource, null);
            holder = new ViewHolder(convertView);
            convertView.setTag(holder);
        } else {
            holder = (ViewHolder) convertView.getTag();
        }
        holder.setInfo(setItem.get(position));
        return convertView;
    }

    public class ViewHolder {
        TextView tvName;
        TextView tvNumberOfNode;


        public ViewHolder(View rootView) {
            tvName = (TextView) rootView.findViewById(R.id.tvName);
            tvNumberOfNode = (TextView) rootView.findViewById(R.id.tvNumberOfNode);
        }

        public void setInfo(SetItem setItem) {
            tvName.setText(setItem.getName());
            tvNumberOfNode.setText(String.valueOf(setItem.getNodeModels().size()));
        }
    }

}
