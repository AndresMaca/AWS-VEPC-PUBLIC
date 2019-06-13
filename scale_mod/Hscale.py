import os
from flask import Flask, request
app = Flask(__name__)

sgw_pool = ['20.0.1.80','20.0.1.158','20.0.1.197','20.0.1.','20.0.1.']

@app.route('/')
def HorizontalScale():
    h_scale = request.args['h_scale']
    cmd = 'sudo ipvsadm -a -t 20.0.0.211:7000 -r ' + sgw_pool[int(h_scale)] + ' -m'
    response = os.popen(cmd).read()
    print(response)
    cmd = 'sudo ipvsadm -a -t 20.0.0.211:7100 -r ' + sgw_pool[int(h_scale)] + ' -m'
    response = os.popen(cmd).read()
    print(response)
    cmd = 'sudo ipvsadm -a -t 20.0.0.211:7200 -r ' + sgw_pool[int(h_scale)] + ' -m'
    response = os.popen(cmd).read()
    print(response)
    return 'lb_sgw_ok'

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080)

