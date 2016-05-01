/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2015 baoyongzhang <baoyz94@gmail.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
package developer.purple.firefly;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.util.TypedValue;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.Toast;

import com.baoyz.swipemenulistview.SwipeMenu;
import com.baoyz.swipemenulistview.SwipeMenuCreator;
import com.baoyz.swipemenulistview.SwipeMenuItem;
import com.baoyz.swipemenulistview.SwipeMenuListView;

import java.util.ArrayList;

import developer.purple.firefly.adapter.NewSetAdapter;
import developer.purple.firefly.manager.DataManager;
import developer.purple.firefly.model.SetItem;
import developer.purple.firefly.utils.DummyDataUtils;


/**
 * Created by dkan10 on 4/5/16.
 */
public class MainActivity extends Activity {
    private SwipeMenuListView mListView;
    private Button btnAddNewSet;
    private WifiManager wifiManager;
    private NewSetAdapter newSetAdapter;
    ArrayList<SetItem> listItems;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        // check SSID
//        checkWifiConnection(getApplicationContext());

        mListView = (SwipeMenuListView) findViewById(R.id.listView);
        btnAddNewSet = (Button) findViewById(R.id.btnAddNewSet);

        listItems = DataManager.getSetList();
//        listItems.add(DummyDataUtils.getDummyUser());

        newSetAdapter = new NewSetAdapter(getApplicationContext(), R.layout.item_list_view, listItems);

        SwipeMenuCreator creator = new SwipeMenuCreator() {

            @Override
            public void create(SwipeMenu menu) {
                // create "delete" item
                SwipeMenuItem deleteItem = new SwipeMenuItem(
                        getApplicationContext());
                // set item background
                deleteItem.setBackground(new ColorDrawable(Color.rgb(0xF9,
                        0x3F, 0x25)));
                // set item width
                deleteItem.setWidth(dpToPx(60));
                // set a icon
                deleteItem.setIcon(R.drawable.ic_delete);
                // add to menu
                menu.addMenuItem(deleteItem);
            }
        };
        // set creator
        mListView.setMenuCreator(creator);
        // set SetAdapter
        mListView.setAdapter(newSetAdapter);

        mListView.setOnMenuItemClickListener(new SwipeMenuListView.OnMenuItemClickListener() {
            @Override
            public boolean onMenuItemClick(int position, SwipeMenu menu, int index) {
                switch (index) {
                    case 0:
                        // Delete button
                        // Remove item at position and notify to adapter
                        listItems.remove(position);
                        newSetAdapter.notifyDataSetChanged();
                        break;
                }
                // false : close the menu; true : not close the menu
                return false;
            }
        });

        mListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                Intent i = new Intent(getApplicationContext(), NodeManagerActivity.class);
                i.putExtra("position", position);
                startActivity(i);
            }
        });

        btnAddNewSet.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent i = new Intent(getApplicationContext(), AddNewSetActivity.class);
                startActivity(i);
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (newSetAdapter != null) {
            listItems = DataManager.getSetList();
            newSetAdapter.notifyDataSetChanged();
        }
    }

    /**
     * @param dp
     * @return int
     * <p/>
     * Change dp to px
     */
    private int dpToPx(int dp) {
        return (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, dp, getResources().getDisplayMetrics());
    }

    /**
     * @param context If SSID connected is Firefly_AP, app continue to work.
     *                Else {Show Toast};
     */
    private void checkWifiConnection(Context context) {
        wifiManager = (WifiManager) getSystemService(context.WIFI_SERVICE);
        // Check Network & AP information
        if (wifiManager.isWifiEnabled() == false) {
            wifiManager.setWifiEnabled(true);
        }
        if (!wifiManager.getConnectionInfo().getSSID().equals("\"Firefly_AP\"")) {
            Toast.makeText(getApplicationContext(),
                    "Required connect to Firefly_AP SSID", Toast.LENGTH_LONG).show();
            finish();
        }
    }
}
