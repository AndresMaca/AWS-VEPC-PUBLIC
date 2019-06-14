from flask import Flask
import os
import json
import time
import requests
app = Flask(__name__)

instance_types = ['t2.small','t2.medium','t2.large','t2.xlarge','t2.2xlarge']
vertical_scale = -1
horizontal_scale = 3
mme_vec = ['i-082a34c9de20e0cb6','i-0082ffbf649261605','i-0ef00d420485c45f3','i-03f73e976de538547','i-078542defe79ec1e5']
sgw_vec = ['i-0097785789f296240','i-058510db5f0700018','i-0cc525d541c5cf67e']
pgw_vec = ['i-0a02951b1b060231c','i-0fae3a714e6939adb','i-031c1ab64ef920b4a']
mme_ids = mme_vec[0] + ' ' + mme_vec[1] + ' ' + mme_vec[2] + ' ' + mme_vec[3] + ' ' + mme_vec[4]
sgw_ids = sgw_vec[0] + ' ' + sgw_vec[1] + ' ' + sgw_vec[2] + ' ' + sgw_vec[3] + ' ' + sgw_vec[4]
pgw_ids = pgw_vec[0] + ' ' + pgw_vec[1] + ' ' + pgw_vec[2] + ' ' + pgw_vec[3] + ' ' + pgw_vec[4]

@app.route('/')
def ScaleDesicion():
    global instance_types
    global vertical_scale
    global horizontal_scale
    global mme_vec
    global sgw_vec
    global pgw_vec
    global mme_ids
    global sgw_ids
    global pgw_ids
    c_users_input = 1.3
    th_input = 1.2
    c_users1 = 1
    c_users2 = 2
    th1 = 1
    th2 = 2
    if c_users_input < c_users1 and th_input < th1:
        #Operacion normal Región I
        print('Region I not need scale!')
    elif c_users_input > c_users1 and th_input >th1 and c_users_input < c_users2 and th_input < th2:
        #Escala Vertical Región II
        if vertical_scale < 5:
            vertical_scale = vertical_scale + 1
        else:
            print('Max vertical scale reached!')
            return 'ok'
        cmd = 'aws ec2 stop-instances --instance-ids '+ mme_ids + ' ' + sgw_ids + ' ' + pgw_ids
        stop_res = os.popen(cmd).read()
        print(stop_res)
        cmd = 'aws ec2 describe-instances --instance-ids '+ mme_ids + ' ' + sgw_ids + ' ' + pgw_ids
        while True:
            wait_res = os.popen(cmd).read()
            if wait_res.find('"Code": 64')==-1:
                 break
            time.sleep(10)
        for mme in mme_vec:
            cmd = 'aws ec2 modify-instance-attribute --instance-id '+ mme + ' --instance-type '+ instance_types[vertical_scale]
            response = os.popen(cmd).read()
            print(response)
        for sgw in sgw_vec:
            cmd = 'aws ec2 modify-instance-attribute --instance-id '+ sgw + ' --instance-type '+ instance_types[vertical_scale]
            response = os.popen(cmd).read()
            print(response)
        for pgw in pgw_vec:
            cmd = 'aws ec2 modify-instance-attribute --instance-id '+ pgw + ' --instance-type '+ instance_types[vertical_scale]
            response = os.popen(cmd).read()
            print(response)
        vertical_instances_to_on = ''
        for i in range(0, horizontal_scale):
            vertical_instances_to_on = vertical_instances_to_on + mme_vec[i] + ' ' + sgw_vec[i] + ' ' + pgw_vec[i] + ' '
        cmd = 'aws ec2 start-instances --instance-ids '+ vertical_instances_to_on
        response = os.popen(cmd).read()
        print(response)
        print('Vertical upgrading done!')
        
    elif c_users_input > c_users2 and th_input > th2:
        #Escala horizontal Región III
        cmd = 'aws ec2 start-instances --instances-ids '+ mme_vec[horizontal_scale]
        response = os.open(cmd).read()
        print(response)
        req1 = request.get('http://3.13.245.230:8080?h_scale='+str(horizontal_scale))
        req2 = request.get('http://3.13.251.239:8080?h_scale='+str(horizontal_scale))
        req3 = request.get('http://3.13.50.60:8080?h_scale='+str(horizontal_scale))
        print('Horizontal upgrading done!')
        horizontal_scale = horizontal_scale + 1
    return 'ok'

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080)
