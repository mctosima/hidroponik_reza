from joblib import dump, load
import tensorflow as tf

ph = 6.5
cahaya = 1
intensitas = 2
suhu = 27.1
ppm = 758
air = 458


sc = load('std_scaler.bin')
SC = load('std_scaler2.bin')

ann0_load = tf.keras.models.load_model("ann0/saved_model.pb")
ann1_load = tf.keras.models.load_model("ann1")
ann2_load = tf.keras.models.load_model("ann2")
ann3_load = tf.keras.models.load_model("ann3")

ANN0_load = tf.keras.models.load_model("ANN-0")
ANN1_load = tf.keras.models.load_model("ANN-1")
ANN2_load = tf.keras.models.load_model("ANN-2")
ANN3_load = tf.keras.models.load_model("ANN-3")
ANN4_load = tf.keras.models.load_model("ANN-4")

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

print(c, C)
