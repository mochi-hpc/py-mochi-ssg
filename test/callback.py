from pyssg.groups import Callback

class MyMembershipCallback(Callback):

    def on_join(self, member):
        print('Member '+str(member.member_id)+' joined')

    def on_left(self, member):
        print('Member '+str(member.member_id)+' left')

    def on_died(self, member):
        print('Member '+str(member.member_id)+' died')

