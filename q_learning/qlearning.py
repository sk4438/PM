from datetime import datetime
from sqlite3 import Timestamp
import numpy as np
import matplotlib.pyplot as plt
import time
import influxdb_client
from influxdb_client.client.write_api import SYNCHRONOUS
import asyncio
import nats

bucket = "pmsb"
org = "nms"
token = "5vgOEDvtHSYxMZLgxj3l2qbpKtmQgBFY2exkYSDGdzvVVaSBB7jsfiv2xUaJzA_NCl4xtaAL9JRmeXITH1rkVQ=="
#token = "583UlunwA5q1klguTpzOPEknmLlGS7eMlklJnkI_UmBOWxKD7xNfbYhT9tDzLdej4cVg9xyVdA-kawaYAalhkQ=="
# Store the URL of your InfluxDB instance
url = "http://localhost:8086"

client = influxdb_client.InfluxDBClient(
    url=url,
    token=token,
    org=org
)

query_api = client.query_api()


async def decision(action):
  nc = nats.NATS()
  async def error_cb(e):
      print("Error:", e)

  async def reconnected_cb():
      print("Connected to NATS")
    
  async def disconnected_cb():
      print("Disconnected!")

  options = {
      "error_cb": error_cb,
      "reconnected_cb": reconnected_cb,
      "disconnected_cb": disconnected_cb,
  }
  options["max_reconnect_attempts"] = -1
  await nc.connect(servers=['nats://localhost:4222','nats://localhost:4222'],**options)
  print("the connection is connected?",nc.is_connected)
  sub = await nc.subscribe("decision")
  data = b"off" if action else b"on"
  await nc.publish("decision",data)
  await nc.close()

class envinorment:
  action_space = 0
  observation_space = 0
  state = 0
  def __init__(self,action_space,observation_space,state):
    self.state = state
    self.action_space = action_space
    self.observation_space = observation_space
  def sample_action(self):
    u = np.random.uniform(0,self.action_space)
    for i in range(1,self.action_space+1):
      if u<i:
        return i-1

  def next(self,action):
    self.state = action
    return self.state


  def cal_reward(self,action,time_stamp):
    thp = np.zeros(4,dtype = float)
    params = {
      "time_stamp":time_stamp
    }
    query = 'from(bucket:"pmsb")\
          |> range(start:time(v: time_stamp))\
          |> filter(fn:(r)=> (r._measurement=="UEThpDl") and ((exists r.QoS) == false) and ((exists r.plmn) == false) and ((exists r.sst) == false) and ((exists r.sd) == false))\
          |> mean()'
                
    results = query_api.query(org=org,query=query,params=params)
    j = 0
    for table in results:
        for record in table.records:
            thp[j] = record.values["_value"]
            j = j + 1
                
    query = 'from(bucket:"pmsb")\
          |> range(start:time(v: time_stamp))\
          |> filter(fn:(r)=> r._measurement=="Energy")\
          |> mean()'
            
    results = query_api.query(org=org,query=query,params=params)
    energy = np.zeros(1,dtype = float)
    j = 0
    for table in results:
        for record in table.records:
          energy[j] = record.values['_value']
          j = j + 1
    avg_thp = np.sum(thp,dtype=float)
    avg_energy = np.sum(energy,dtype = float)
    print("avg_thp: " + str(avg_thp))
    print("avg_energy: " + str(avg_energy))
    print(str(self.state) + " "+str(action))
    if action == 0:
      print("reward : " + str(0.5*(avg_thp-avg_energy)))
      return 0.5*(avg_thp-avg_energy)
    else:
      print("reward : " + str(0.5*(avg_energy-avg_thp)))
      return 0.5*(avg_energy-avg_thp)


env = envinorment(2,2,0)

#parameters
learning_rate = 0.2
discount_rate = 0.99
min_exploration_rate = 0.01
max_exploration_rate = 1
exploration_decay = 0.01


#stats
noOfexplored = 0
noOfexploted = 0

transition = []

EE = []
rewards = []

for i in range(100):
  #Q_table
  Q_table = np.loadtxt("Qtable")
  param = np.loadtxt("param")
  prev_state = int(param[0])
  env.state = prev_state
  exploration_rate = param[1]
  epno = int(param[2])
  prev_act = int(param[3])
  time_stamp = param[4]

  if prev_act!=-1:
    #Reward calculation
    reward = env.cal_reward(prev_act,int(time_stamp))

    #state transition
    curr_state = env.next(prev_act)

    #Q_table updation
    Q_table[prev_state][prev_act] = (1-learning_rate)*Q_table[prev_state][prev_act] + learning_rate*(reward + discount_rate*Q_table[curr_state][np.argmax(Q_table[curr_state])])
    
    #Exploration decay
    exploration_rate = min_exploration_rate + (max_exploration_rate - min_exploration_rate)*np.exp(-exploration_decay*(epno))

    #stats
    rewards.append(reward)
    transition.append(curr_state)
  else:
    curr_state = prev_state
    transition.append(curr_state)


  #training
  ep = np.random.uniform(0,1)
  #exploration vs explotitation
  if ep < exploration_rate:
    action = env.sample_action()
    noOfexplored = noOfexplored + 1
  else :
    action = np.argmax(Q_table[curr_state])
    noOfexploted = noOfexploted + 1
  timestamp = time.time_ns()
  asyncio.run(decision(action))
  time.sleep(20)
  param[0] = curr_state
  param[1] = exploration_rate
  param[2] = epno + 1
  param[3] = action
  param[4] = timestamp
  

  np.savetxt("Qtable",Q_table)
  np.savetxt("param",param)


#Q_table
Q_table = np.loadtxt("Qtable")
param = np.loadtxt("param")
#timestmp = np.loadtxt("timestamp")
prev_state = int(param[0])
env.state = prev_state
exploration_rate = param[1]
epno = int(param[2])
prev_act = int(param[3])
time_stamp = param[4]

if prev_act!=-1:
    #Reward calculation
    reward = env.cal_reward(prev_act,int(time_stamp))

    #state transition
    curr_state = env.next(prev_act)

    #Q_table updation
    Q_table[prev_state][prev_act] = (1-learning_rate)*Q_table[prev_state][prev_act] + learning_rate*(reward + discount_rate*Q_table[curr_state][np.argmax(Q_table[curr_state])])
    
    #Exploration decay
    exploration_rate = min_exploration_rate + (max_exploration_rate - min_exploration_rate)*np.exp(-exploration_decay*(epno))

    #stats
    rewards.append(reward)
    transition.append(curr_state)
else:
    curr_state = prev_state
    transition.append(curr_state)


for i in range(10):
  num = 0
  dem = 0
  for j in range(10):
    num = num + rewards[i*10 + j]
    dem = dem + abs(rewards[i*10 + j])
  print("i = " + str(i) + ": " + str(num/dem))


print("times in exploration : " + str(noOfexplored))
print("times in explotiation: " + str(noOfexploted))


plt.plot(range(1,102),rewards)
plt.xlabel("iteration")
plt.ylabel("Reward")

plt.plot(range(1,101),np.zeros(100))
plt.show()

'''
plt.plot(range(1,101),con)
plt.xlabel("iteration")
plt.ylabel("cummilative Reward")
plt.show()


plt.plot(range(1,301),Energysaved)
plt.xlabel("iteration")
plt.ylabel("Energy saved")
plt.show()

plt.plot(range(1,301),thpinsaving,label="in")
plt.legend()
plt.xlabel("iteration")
plt.ylabel("Thp")

plt.plot(range(1,301),thpoutsaving,label="out")
plt.legend()
plt.xlabel("iteration")
plt.ylabel("Thp")
plt.show()
'''

plt.plot(range(1,102),transition)
plt.xlabel("iteration")
plt.ylabel("transititon")
plt.show()
