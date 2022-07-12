from scipy import stats
import matplotlib.pyplot as plt
import numpy as np

corr =[[1,0.5],[0.5,1]]

def gamma():
    mvnorm = stats.multivariate_normal(mean=[0,0],cov=corr)
    norm = stats.norm()
    #print(sam_unif)
    cov_lognm = stats.lognorm(s=0.5,scale=3000)
    cov_lognm1 = stats.lognorm(s=0.5,scale=10000)
    thp = []
    enr = []
    x = []
    for i in range(1000):
        sam = mvnorm.rvs(1)
        x.append(i+1)
        sam_unif = norm.cdf(sam)
        thp.append(cov_lognm.ppf(sam_unif[0]))
        enr.append(cov_lognm1.ppf(sam_unif[1]))
    
    plt.scatter(x,thp)
    plt.show()

    plt.scatter(x,enr)
    plt.show()


def gamm():
    mvnorm = stats.multivariate_normal(mean=[0,0],cov=corr)
    norm = stats.norm()
    #print(sam_unif)
    cov_lognm = stats.gamma(3000)
    cov_lognm1 = stats.gamma(10000)
    thp = []
    enr = []
    x = []
    for i in range(1000):
        sam = mvnorm.rvs(1)
        x.append(i+1)
        sam_unif = norm.cdf(sam)
        thp.append(cov_lognm.ppf(sam_unif[0]))
        enr.append(cov_lognm1.ppf(sam_unif[1]))
    
    plt.scatter(x,thp)
    plt.show()

    plt.scatter(x,enr)
    plt.show()


gamma()
gamm()