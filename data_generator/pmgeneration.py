from scipy import stats
import time
import numpy as np
import asyncio
import nats
import matplotlib.pyplot as plt


noOfDus = 4
noOfQoseachDu = 10
noOfgNb = 1
noOfcoveachgNb = 3
noOfsmalleachgNb = 1

state = 0
corr_off = np.identity(noOfcoveachgNb*noOfgNb*noOfQoseachDu + noOfgNb,dtype=float)
corr_on = np.identity(noOfDus*noOfQoseachDu + noOfgNb,dtype=float)
covcell_corrbtthpenr = 0.18
smallcell_corrbtthpenr = 0.1
smallcell_covcell = -0.1


for i in range(noOfgNb):
  for j in range(i*noOfcoveachgNb*noOfQoseachDu,(i+1)*noOfcoveachgNb*noOfQoseachDu):
    corr_off[j][noOfcoveachgNb*noOfgNb*noOfQoseachDu + i] = covcell_corrbtthpenr
    corr_off[noOfcoveachgNb*noOfgNb*noOfQoseachDu + i][j] = covcell_corrbtthpenr
    corr_on[j][noOfcoveachgNb*noOfgNb*noOfQoseachDu + i] = covcell_corrbtthpenr - 0.02
    corr_on[noOfcoveachgNb*noOfgNb*noOfQoseachDu + i][j] = covcell_corrbtthpenr - 0.02

for i in range(noOfsmalleachgNb*noOfgNb):
  for j in range(3*noOfQoseachDu):
    corr_on[i*3*noOfQoseachDu + j][noOfcoveachgNb*noOfgNb*noOfQoseachDu + noOfgNb + i*noOfQoseachDu + j%noOfQoseachDu] = smallcell_covcell
    corr_on[noOfcoveachgNb*noOfgNb*noOfQoseachDu + noOfgNb + i*noOfQoseachDu + j%noOfQoseachDu][i*3*noOfQoseachDu + j] = smallcell_covcell

for i in range(noOfsmalleachgNb*noOfgNb):
  for j in range(noOfQoseachDu):
    corr_on[noOfcoveachgNb*noOfgNb*noOfQoseachDu + noOfgNb + i*noOfQoseachDu + j%noOfQoseachDu][noOfcoveachgNb*noOfgNb*noOfQoseachDu + int(i/noOfsmalleachgNb)] = smallcell_corrbtthpenr
    corr_on[noOfcoveachgNb*noOfgNb*noOfQoseachDu + int(i/noOfsmalleachgNb)][noOfcoveachgNb*noOfgNb*noOfQoseachDu + noOfgNb + i*noOfQoseachDu + j%noOfQoseachDu] = smallcell_corrbtthpenr



#checking whether the correlation matrix is PD
print(np.all(np.linalg.eigvals(corr_on)>0))
print(np.all(np.linalg.eigvals(corr_off)>0))

noofunits_off = noOfcoveachgNb*noOfgNb*noOfQoseachDu + noOfgNb
noofunits_on = noOfDus*noOfQoseachDu + noOfgNb

#heavy load
async def gamma_high(mvnorm_on,mvnorm_off):
  for i in range(5000):
    nc = nats.NATS()
    async def error_cb(e):
       print(f"There is an error : {e}")

    async def reconnected_cb():
        print(f"Connected to NATS at {nc.connected_url.netloc}...")
    
    async def disconnected_cb():
        print("Disconnected!")

    options = {
        "error_cb": error_cb,
        "reconnected_cb": reconnected_cb,
        "disconnected_cb": disconnected_cb
    }
    #options["max_reconnect_attempts"] = -1
    await nc.connect(servers=['nats://localhost:4222'],**options)
    print("the connection is connected?",nc.is_connected)
    if state == 0:
      sam = mvnorm_on.rvs(1)
      norm = stats.norm()
      sam_unif = norm.cdf(sam)
      #print(sam_unif)
      cov_gamma = stats.gamma(10000)
      small_gamma = stats.gamma(11000)
      cov_gamma1 = stats.gamma(40000)
      samp = "0 "
      for i in range(noofunits_on):
        if i>=noofunits_off-noOfgNb and i<noofunits_off: 
          val = cov_gamma1.ppf(sam_unif[i])
        elif i>=noofunits_off:
          val = small_gamma.ppf(sam_unif[i])
        else:
          val = cov_gamma.ppf(sam_unif[i])
        samp = samp + str(int(val)) + " "
      sub = await nc.subscribe("foo")
      await nc.publish("foo",samp.encode())
      print("published gamma_high")
      await nc.close()
      await asyncio.sleep(2)
    else:
      sam = mvnorm_off.rvs(1)
      norm = stats.norm()
      sam_unif = norm.cdf(sam)
      cov_gamma = stats.gamma(10000)
      cov_gamma1 = stats.gamma(29000)
      samp = "1 "
      for i in range(noofunits_off):
        if i>=noofunits_off-noOfgNb and i<noofunits_off: 
          val = cov_gamma1.ppf(sam_unif[i])
        else:
          val = cov_gamma.ppf(sam_unif[i])
        samp = samp + str(int(val)) + " "
      sub = await nc.subscribe("foo")
      await nc.publish("foo",samp.encode())
      print("published gamma_high")
      await nc.close()
      await asyncio.sleep(2)



#moderate load
async def lognorm(mvnorm_on,mvnorm_off):
  for i in range(5000):
    nc = nats.NATS()
    async def error_cb(e):
        print(f"There is an error : {e}")

    async def reconnected_cb():
        print(f"Connected to NATS at {nc.connected_url.netloc}...")
    
    async def disconnected_cb():
        print("Disconnected!")

    options = {
        "error_cb": error_cb,
        "reconnected_cb": reconnected_cb,
        "disconnected_cb": disconnected_cb
    }
    #options["max_reconnect_attempts"] = -1
    await nc.connect(servers=['nats://localhost:4222'],**options)
    print("the connection is connected?",nc.is_connected)
    print(state)
    if state == 0:
      sam = mvnorm_on.rvs(1)
      norm = stats.norm()
      sam_unif = norm.cdf(sam)
      #print(sam_unif)
      log = stats.lognorm(s=0.5,scale = 10000)
      log1 = stats.lognorm(s=0.5,scale = 40000)
      samp = "0 "
      for i in range(noofunits_on):
        if i>=noofunits_off-noOfgNb and i<noofunits_off : 
          val = log1.ppf(sam_unif[i])
        else:
          val = log.ppf(sam_unif[i])
        samp = samp + str(int(val)) + " "
      sub = await nc.subscribe("foo")
      await nc.publish("foo",samp.encode())
      print("published lognormal")
      await nc.close()
      await asyncio.sleep(2)
    else:
      sam = mvnorm_off.rvs(1)
      norm = stats.norm()
      sam_unif = norm.cdf(sam)
      #print(sam_unif)
      log = stats.lognorm(s = 0.5,scale = 10000)
      log1 = stats.lognorm(s = 0.5,scale = 30000)
      samp = "1 "
      for i in range(noofunits_off):
        if i>=noofunits_off-noOfgNb and i<noofunits_off : 
          val = log1.ppf(sam_unif[i])
        else:
          val = log.ppf(sam_unif[i])
        samp = samp + str(int(val)) + " "
      sub = await nc.subscribe("foo")
      await nc.publish("foo",samp.encode())
      print("published lognormal")
      await nc.close()
      await asyncio.sleep(2)

#low load
async def gamma_low(mvnorm_on,mvnorm_off):
  for i in range(5000):
    nc = nats.NATS()
    async def error_cb(e):
        print(f"There is an error : {e}")

    async def reconnected_cb():
        print(f"Connected to NATS")
    
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
    print(state)
    if state == 0:
      sam = mvnorm_on.rvs(1)
      norm = stats.norm()
      sam_unif = norm.cdf(sam)
      cov_gamma = stats.gamma(3000)
      small_gamma = stats.gamma(3000)
      cov_gamma1 = stats.gamma(13000)
      samp = "0 "
      for i in range(noofunits_on):
        if i>=noofunits_off-noOfgNb and i<noofunits_off: 
          val = cov_gamma1.ppf(sam_unif[i])
        elif i>=noofunits_off:
          val = small_gamma.ppf(sam_unif[i])
        else:
          val = cov_gamma.ppf(sam_unif[i])
        samp = samp + str(int(val)) + " "
      sub = await nc.subscribe("foo")
      await nc.publish("foo",samp.encode())
      print("published gamma_low")
      await nc.close()
      await asyncio.sleep(2)
    else:
      sam = mvnorm_off.rvs(1)
      norm = stats.norm()
      sam_unif = norm.cdf(sam)
      #print(sam_unif)
      cov_gamma = stats.gamma(3000)
      cov_gamma1 = stats.gamma(10000)
      samp = "1 "
      for i in range(noofunits_off):
        if i>=noofunits_off-noOfgNb and i<noofunits_off: 
          val = cov_gamma1.ppf(sam_unif[i])
        else:
          val = cov_gamma.ppf(sam_unif[i])
        samp = samp + str(int(val)) + " "
      sub = await nc.subscribe("foo")
      await nc.publish("foo",samp.encode())
      print("published gamma_low")
      await nc.close()
      await asyncio.sleep(2)



async def guassgen():
  mvnorm_on = stats.multivariate_normal(mean=np.zeros(noofunits_on),cov=corr_on)
  mvnorm_off = stats.multivariate_normal(mean=np.zeros(noofunits_off),cov=corr_off)
  i = 1
  while True:
    if i==1:
      await gamma_high(mvnorm_on,mvnorm_off)
    elif i==2:
      await gamma_low(mvnorm_on,mvnorm_off)
    else:
      await lognorm(mvnorm_on,mvnorm_off)
    i = (i+1)%3

async def qlearning_des():
  while True:
    nc = nats.NATS() 
    async def error_cb(e):
        print(f"There is an error : {e}")

    async def reconnected_cb():
        print(f"Connected to NATS")
      
    async def disconnected_cb():
        print("Disconnected!")

    options = {
        "error_cb": error_cb,
        "reconnected_cb": reconnected_cb,
        "disconnected_cb": disconnected_cb,
    }
    options["max_reconnect_attempts"] = -1
    await nc.connect(servers=['nats://localhost:4222','nats://localhost:4222'],**options)
    print("the connection is connected? on/off ",nc.is_connected)
    async def msg_handler(msg):
      dt = msg.data.decode()
      global state
      state = 0 if dt=="on" else 1
      print("current state : " + str(state))
    await nc.subscribe("decision",'',msg_handler)
    await asyncio.sleep(20)
    await nc.close()

    

async def main():
  f1 = asyncio.create_task(guassgen())
  f2 = asyncio.create_task(qlearning_des())
  await asyncio.wait([f1,f2])


asyncio.run(main())