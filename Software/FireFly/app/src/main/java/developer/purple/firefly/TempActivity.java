package developer.purple.firefly;

import android.app.ListActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.EditText;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by dkan10 on 4/11/16.
 */
public class TempActivity extends ListActivity {
    EditText et;
    String listItem[] = {"Dell Inspiron", "HTC One X", "HTC Wildfire S", "HTC Sense", "HTC Sensation XE"};


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.temp_layout);
        et = (EditText) findViewById(R.id.editText);

        List values = new ArrayList();
        for (int i = 0; i < listItem.length; i++) {
            values.add(listItem[i]);
        }

        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1, values);
        setListAdapter(adapter);
    }

    public void onClick(View view) {
        ArrayAdapter<String> adapter = (ArrayAdapter<String>) getListAdapter();
        String device;
        switch (view.getId()) {
            case R.id.addItem:
                List myList = new ArrayList();
                device = et.getText().toString();
                myList.add(device);
                adapter.add(device);
                et.setText("");
                break;
            case R.id.exit:
                finish();
                break;
        }
        adapter.notifyDataSetChanged();
    }
}