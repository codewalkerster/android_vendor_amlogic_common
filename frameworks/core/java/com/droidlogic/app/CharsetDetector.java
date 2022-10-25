/*
 *  Copyright 2010 Georgios Migdos <cyberpython@gmail.com>.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *  under the License.
 */

package com.droidlogic.app;

import java.io.BufferedInputStream;
import java.io.EOFException;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.ByteBuffer;
import java.nio.charset.CharacterCodingException;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;

import android.util.Log;

/**
 *
 * @author
 */
public class CharsetDetector {
    private static final String TAG = "CharsetDetector";

    private static final int UTF8_FLAG    = 0xefbb;
    private static final int UNICODE_FLAG = 0xfffe;
    private static final int UTF16BE_FLAG = 0xfeff;
    private static final int ANSI_FLAG    = 0x5c75;
    private static final int BIG5_FLAG    = 0x310d;
    private static final int GB2312_FLAG  = 0x5b53;


    String[] mCharsetsToBeTested = {
        "UTF8",
        "GB2312",
        "GBK",
        "cp932",
        "cp949",
        "cp874",
        "cp1255",
        "Windows-1256",
        "cp1250",
        "Big5",
        "cp1254",
        "cp1098",
        "ISO-8859-1",
        "ISO-8859-2",
        "ISO-8859-5",
        "ISO-8859-6",
        "ISO-8859-7",
        "ISO-8859-8",
        "UTF-16BE",
        "UTF-16LE",
        "UTF-32BE",
        "UTF-32LE",
        "Shift_JIS",
        "ISO-2022-JP",
        "ISO-2022-CN",
        "ISO-2022-KR",
        "GB18030",
        "EUC-JP",
        "EUC-KR",
        "Windows-1251",
        "Windows-1256",
        "KOI8-R",
        "ISO-8859-9",
        "IBM424_ltr",
        "IBM424_rtr",
        "IBM420_rtr",
        "IBM420_ltr"
    };


    public Charset detectCharset(File f) {

        Charset charset = null;
        int len = 0;
        String fileDetect = null;

        fileDetect = detectCharsetFromFilePath(f);
        if (fileDetect != null) {
            Log.i(TAG,"fileDetect:"+fileDetect);
            return Charset.forName(fileDetect);
        }

        len = getFileSize(f);
        //Log.i(TAG,"file size:" + len);
        if (len <= 0) {
            Log.e(TAG,"file is not valid!");
            return null;
        }

        for (String charsetName : mCharsetsToBeTested) {
            charset = detectCharset(f, Charset.forName(charsetName), len);
            if (charset != null) {
                break;
            }
        }
        Log.i(TAG,"charsetName:"+charset);
        return charset;
    }

    public static int getFileSize(File file) {
        if (!file.exists() || !file.isFile()) {
            Log.e(TAG,"file is not exist!!");
            return 0;
        }
        return (int)file.length();

    }

    public static String detectCharsetFromFilePath(File file){
        if (!file.exists() || !file.isFile()) {
            Log.e(TAG,"file is not exist!!");
            return null;
        }

        String pathName = file.getName();
        if (pathName == null) {
            return null;
        }

        Log.i(TAG,"detectCharsetFromFilePath pathName:" + pathName);

        if (pathName.contains("8859-1") || pathName.contains("8859_1")) {
            return "iso-8859-1";
        }
        if (pathName.contains("Windows-1250") || pathName.contains("1250")) {
            return "Windows-1250";
        }
        if (pathName.contains("Windows-1251") || pathName.contains("1251")) {
            return "Windows-1251";
        }
        if (pathName.contains("Windows-1252") || pathName.contains("1252")) {
            return "Windows-1252";
        }
        if (pathName.contains("Windows-1253") || pathName.contains("1253")) {
            return "Windows-1253";
        }
        if (pathName.contains("Windows-1254") || pathName.contains("1254")) {
            return "Windows-1254";
        }
        if (pathName.contains("Windows-1255") || pathName.contains("1255")) {
            return "Windows-1255";
        }
        if (pathName.contains("Windows-1256") || pathName.contains("1256")) {
            return "Windows-1256";
        }
        if (pathName.contains("Windows-1257") || pathName.contains("1257")) {
            return "Windows-1257";
        }

        if (pathName.contains("Windows-874") || pathName.contains("CP874")) {
            return "x-IBM874";
        }

        String code = null;
        try {
            BufferedInputStream bin = new BufferedInputStream(new FileInputStream(file));
            int p = (bin.read() << 8) + bin.read();
            bin.close();
            switch (p) {
                case UTF8_FLAG:
                    code = "UTF-8";
                    break;
                case UNICODE_FLAG:
                    code = "Unicode";
                    break;
                case UTF16BE_FLAG:
                    code = "UTF-16BE";
                    break;
                case GB2312_FLAG:
                    code = "GB2312";
                    break;
                case ANSI_FLAG:
                    code = "ANSI|ASCII" ;
                    break;
            }
        } catch (Exception e) {
            return null;
        }

        return code != null ? code : null;
    }

    private Charset detectCharset(File f, Charset charset, int len) {
        try {
            BufferedInputStream input = new BufferedInputStream(new FileInputStream(f));

            Log.i (TAG,"[detectCharset]len:" + len);
            CharsetDecoder decoder = charset.newDecoder();
            decoder.reset();

            byte[] buffer = new byte[len];
            boolean identified = false;
            while ((input.read(buffer) != -1) && (!identified)) {
                identified = identify(buffer, decoder);
            }

            input.close();

            if (identified) {
                return charset;
            } else {
                return null;
            }

        } catch (Exception e) {
            return null;
        }
    }

    private boolean identify(byte[] bytes, CharsetDecoder decoder) {
        try {
            decoder.decode(ByteBuffer.wrap(bytes));
        } catch (CharacterCodingException e) {
            return false;
        }
        return true;
    }
}

