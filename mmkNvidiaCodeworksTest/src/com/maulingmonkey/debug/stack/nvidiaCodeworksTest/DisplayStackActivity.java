/* Copyright 2017 MaulingMonkey

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

package com.maulingmonkey.debug.stack.nvidiaCodeworksTest;

import android.app.Activity;
import android.text.method.ScrollingMovementMethod;
import android.os.Bundle;
import android.widget.ScrollView;
import android.widget.TextView;

public class DisplayStackActivity extends Activity {
	static {
		System.loadLibrary("mmkNvidiaCodeworksTest");
	}

	private ScrollView sv;
	private TextView tv;

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		mmkDebugStackSearchPaths(getApplicationInfo().nativeLibraryDir);
		//mmkDebugStackSearchPaths(ActivityThread.systemMain().getSystemContext().getApplicationInfo().nativeLibraryDir); // Requires higher android api level than I'm setting?

		StringBuilder sb = new StringBuilder();
		sb.append(traceDemo());
		//sb.append("\n\n");
		//sb.append(inspectElf(getApplicationInfo().nativeLibraryDir, System.mapLibraryName("mmkNvidiaCodeworksTest")));

		tv = new TextView(this);
		tv.setText(sb.toString());

		sv = new ScrollView(this);
		sv.addView(tv);
		setContentView(sv);
	}

	public native void mmkDebugStackSearchPaths(String elfDir);
	public native String traceDemo();
	public native String inspectElf(String elfDir, String elfName);
}
