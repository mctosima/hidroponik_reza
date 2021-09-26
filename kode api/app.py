from flask import Flask, request, jsonify
import mysql.connector
import os
import pymysql
import sys
from joblib import dump, load
import tensorflow as tf


app = Flask(__name__)
 
#  ubah ini sesuai config an db
db = mysql.connector.connect(
  host="127.0.0.1",
  port="3306",
  user="hidroponik",
  password="Recchan131099",
  database="hidroponik"
)

import requests

bot_token = '1670809057:AAE591XDoAKUG4FZc_5grFaeBh9-LptcwVE'
bot_chatID = '1334842344'
def sendNotification(bot_message):
    send_text = 'https://api.telegram.org/bot' + bot_token + '/sendMessage?chat_id=' + bot_chatID + '&parse_mode=Markdown&text=' + bot_message
    response = requests.get(send_text)
    return response.json()


conn = db.cursor(buffered=True , dictionary=True)

if __name__ == '__main__':
    app.run(host="0.0.0.0",port="5000", debug=True)

# send data
@app.route('/data', methods=['POST'])
def add_data():
    # simpen semua data di variable json
    _json = request.get_json(force=True)
    print("Request nya:", file=sys.stderr)
    print(request, file=sys.stderr)
    # ambil semua data dari parameter
    tds = _json['tds']
    ph_meter = _json['ph_meter']
    temp = _json['temp']
    light = _json['light']
    water = _json['water']
    water_flow = _json['water_flow']

    # masukin ke variable data semuanya (note emang butuh , di akhirannya)
    data = (tds, ph_meter, temp, water, water_flow, light,)

    # buat query, dalem values harus %s
    query = "INSERT INTO data_node (tds, ph_meter, temp, water, water_flow, cahaya) values (%s, %s, %s, %s, %s, %s)"
    
    # execute query
    conn.execute(query, data)
    # insert data
    db.commit()

    # get data yang tadi di create buat ditampilin di response
    insertID = conn.lastrowid
    query = "SELECT * FROM data_node where id = %s"
    conn.execute("SELECT * FROM data_node where id = %s", (insertID,))
    insertData = conn.fetchone()
    
    ml(ph_meter, light, water_flow, temp, tds, water)

    # return response jsonnya
    return jsonify({
        "message": "Created",
        "data" : insertData,
        "code" : 201
    }), 201

# ini method untuk dapetin semuanya
@app.route('/data', methods=['GET'])
def get_all_data():
    query = "SELECT * FROM data_node"
    conn.execute(query)
    rows = conn.fetchall()
    return jsonify({
        "message": "Fetch All",
        "data" : rows,
        "code" :200
    }), 200

# ini method untuk dapetin semuanya
@app.route('/data/last', methods=['GET'])
def get_last_data():
    query = "SELECT * FROM data_node order by id desc"
    conn.execute(query)
    rows = conn.fetchall()
    return jsonify({
        "message": "Fetch Last ID",
        "data" : rows[0],
        "code" :200
    }), 200

def add_data_result(for_mcu, for_notification):
    # masukin ke variable data semuanya (note emang butuh , di akhirannya)
    data = (for_mcu, for_notification,)
    # buat query, dalem values harus %s
    query = "INSERT INTO data_result (for_mcu, for_notification) values (%s, %s)"
    # execute query
    conn.execute(query, data)
    # insert data
    db.commit()
    # message
    if for_notification == 0:
        text = "Kondisi ku baik-baik saja, tetap perhatikan aku yaa"
    elif for_notification == 1:
        text = "Waduh tong air harus di tambah air nih, soalnya ph terlalu rendah/tinggi, atau ppm terlalu tinggi, atau air dalam tong habis"
    elif for_notification == 2:
        text = "Hai! Pompa air mu mati, nanti tanamannya layu loh!"
    elif for_notification == 3:
        text="Halo! Pompa air mu terlalu kencang nih nanti tanamanmu tidak sehat loh!"
    elif for_notification == 4:
        text = "Aduh panas banget nih, tanamanmu butuh udara sejuk"
    sendNotification(text)


    # get data yang tadi di create buat ditampilin di response
    # insertID = conn.lastrowid
    # query = "SELECT * FROM data_result where id = %s"
    # conn.execute("SELECT * FROM data_result where id = %s", (insertID,))
    # insertData = conn.fetchone()

@app.route('/data-ml/last', methods=['GET'])
def get_result_data_last():
    query = "SELECT * FROM data_result order by id desc limit 1"
    conn.execute(query)
    rows = conn.fetchall()
    return jsonify({
        "message": "Fetch Last Id",
        "data" : rows[0],
        "code" :200
    }), 200

# kode bagian ml ini modelnya diubah disesuaikan dengan algoritma berapa yang dipakai, 
# bagian ini masih menggunakan algoritma 1
def ml(ph, cahaya, intensitas, suhu, ppm, air):
    sc = load('models/std_scaler.bin')
    SC = load('models/std_scaler2.bin')

    ann0_load = tf.keras.models.load_model("models/ann0")
    ann1_load = tf.keras.models.load_model("models/ann1")
    ann2_load = tf.keras.models.load_model("models/ann2")
    ann3_load = tf.keras.models.load_model("models/ann3")

    ANN0_load = tf.keras.models.load_model("models/ANN-0")
    ANN1_load = tf.keras.models.load_model("models/ANN-1")
    ANN2_load = tf.keras.models.load_model("models/ANN-2")
    ANN3_load = tf.keras.models.load_model("models/ANN-3")
    ANN4_load = tf.keras.models.load_model("models/ANN-4")

    prediction0 = ann0_load.predict(sc.transform([[cahaya,
                                                intensitas,
                                                suhu,
                                                ppm]]))

    print(prediction0)
    if (prediction0 > 0.5):
        print('ga ngapa2in 1')
    else:
        print('ngapain 0')

    prediction1 = ann1_load.predict(sc.transform([[cahaya,
                                                intensitas,
                                                suhu,
                                                ppm]]))

    print(prediction1)
    if (prediction1 > 0.5):
        print('ngapain lampu 1')
    else:
        print('ngapain lain 0')

    prediction2 = ann2_load.predict(sc.transform([[cahaya,
                                                intensitas,
                                                suhu,
                                                ppm]]))

    print(prediction2)
    if (prediction2 > 0.5):
        print('ngapain pompa 1')
    else:
        print('ngapain lain 0')

    prediction3 = ann3_load.predict(sc.transform([[cahaya,
                                                intensitas,
                                                suhu,
                                                ppm]]))

    print(prediction3)
    if (prediction3 > 0.5):
        print('ngapain pompa dan lampu 1')
    else:
        print('ngapain lain 0')

    a = [prediction0, prediction1, prediction2, prediction3]
    b = max(a)
    c = a.index(b)
    print(b)
    print(c)

    Prediction0 = ANN0_load.predict(SC.transform([[ph,
                                                cahaya,
                                                intensitas,
                                                suhu,
                                                ppm,
                                                air]]))

    print(Prediction0)
    if (Prediction0 > 0.5):
        print('ga ngapa2in 1')
    else:
        print('ngapain 0')

    Prediction1 = ANN1_load.predict(SC.transform([[ph,
                                                cahaya,
                                                intensitas,
                                                suhu,
                                                ppm,
                                                air]]))

    print(Prediction1)
    if (Prediction1 > 0.5):
        print('ngapain NOTIF 1 1')
    else:
        print('ngapain 0')

    Prediction2 = ANN2_load.predict(SC.transform([[ph,
                                                cahaya,
                                                intensitas,
                                                suhu,
                                                ppm,
                                                air]]))

    print(Prediction2)
    if (Prediction2 > 0.5):
        print('ngapain NOTIF 2 1')
    else:
        print('ngapain 0')

    Prediction3 = ANN3_load.predict(SC.transform([[ph,
                                                cahaya,
                                                intensitas,
                                                suhu,
                                                ppm,
                                                air]]))

    print(Prediction3)
    if (Prediction3 > 0.5):
        print('ngapain NOTIF 3 1')
    else:
        print('ngapain 0')

    Prediction4 = ANN4_load.predict(SC.transform([[ph,
                                                cahaya,
                                                intensitas,
                                                suhu,
                                                ppm,
                                                air]]))

    print(Prediction4)
    if (Prediction4 > 0.5):
        print('ngapain NOTIF 4 1')
    else:
        print('ngapain 0')

    A = [Prediction0, Prediction1, Prediction2, Prediction3, Prediction4]
    B = max(A)
    C = A.index(B)
    print(B)
    print(C)
    print("ml: ", c, " notif: ", C)
    add_data_result(c,C)
