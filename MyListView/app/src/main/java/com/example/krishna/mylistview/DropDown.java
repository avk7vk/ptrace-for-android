package com.example.krishna.mylistview;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;

import java.util.ArrayList;


public class DropDown  implements View.OnClickListener {

    protected Button selectColoursButton;

    private LayoutInflater inflater;
    private Activity activity;

    protected CharSequence[] colours = { "Red", "Green", "Blue", "Yellow", "Orange", "Purple" };

    protected ArrayList<CharSequence> selectedColours = new ArrayList<CharSequence>();

    public DropDown(LayoutInflater inflater, Activity activity) {
        this.inflater = inflater;
        this.activity = activity;
        View view = inflater.inflate(R.layout.activity_track_sys_calls, null);
//        selectColoursButton = (Button) view.findViewById(R.id.select_colours);
        selectColoursButton.setOnClickListener(this);

    }
    @Override
    public void onClick(View view) {
        switch(view.getId()) {
  //          case R.id.select_colours:
     //           showSelectColoursDialog();
       //         break;
            default:
                break;
        }
    }
    public void setInflater(LayoutInflater inflater, Activity activity) {
        this.inflater = inflater;
        this.activity = activity;
    }

    protected void showSelectColoursDialog() {

        boolean[] checkedColours = new boolean[colours.length];
        int count = colours.length;
        for(int i = 0; i < count; i++)
            checkedColours[i] = selectedColours.contains(colours[i]);
        DialogInterface.OnMultiChoiceClickListener coloursDialogListener = new DialogInterface.OnMultiChoiceClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which, boolean isChecked) {
                if(isChecked)
                    selectedColours.add(colours[which]);
                else
                    selectedColours.remove(colours[which]);
                onChangeSelectedColours();
            }
        };

        AlertDialog.Builder builder = new AlertDialog.Builder(activity.getApplication().getApplicationContext());
        builder.setTitle("Select Colours");
        builder.setMultiChoiceItems(colours, checkedColours, coloursDialogListener);
        AlertDialog dialog = builder.create();
        dialog.show();

    }
    protected void onChangeSelectedColours() {
        StringBuilder stringBuilder = new StringBuilder();
        for(CharSequence colour : selectedColours)
            stringBuilder.append(colour + ",");
        selectColoursButton.setText(stringBuilder.toString());

    }

}
