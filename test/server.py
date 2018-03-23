from pymargo import MargoInstance
import pyssg

mid = MargoInstance('tcp')
pyssg.init(mid)
pyssg.finalize()
