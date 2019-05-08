import sys
sys.path.append('./gen')

from gen.bank import StandardService, PremiumService, ManageService
from gen.bank.ttypes import FailedLoginException, FailedToCreateException, FailedOperationException
from gen.bank.ttypes import CurrencyName, MonetaryType, ClientData, AccountType, AccountInfo


from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol.TBinaryProtocol import TBinaryProtocolFactory
from thrift.server import TServer
from thrift.TMultiplexedProcessor import TMultiplexedProcessor
import threading
import hashlib
import time
import math

from exchange_rates_client import subscribe

_ONE_HOUR_IN_SECONDS = 60 * 60


class ManageHandler(object):
    def __init__(self, accounts, accounts_lock):
        self.accounts = accounts
        self.accounts_lock = accounts_lock

    def getAccount(self, client_data):
        print('Creating account: ', client_data.PESEL, client_data.income, client_data.first_name, client_data.last_name)
        if client_data.PESEL == '0':
            raise FailedToCreateException("Wrong PESEL!")

        new_account = Account(client_data.PESEL, client_data.first_name, client_data.last_name, client_data.income)

        self.accounts_lock.acquire()
        if client_data.PESEL in self.accounts:
            self.accounts_lock.release()
            raise FailedToCreateException("Account for this PESEL already exists!")
        else:
            self.accounts[client_data.PESEL] = new_account
            self.accounts_lock.release()

        # not elegant, not sure if _password can be read without a lock...
        return new_account.info


class Account(object):

    def __init__(self, PESEL,first_name, last_name, income):
        self.PESEL = PESEL
        self.first_name = first_name
        self.last_name = last_name
        self.balance = MonetaryType(0, 0)
        # placeholder...
        password = self.get_new_password(PESEL)
        if income > 6000:
            self.info = AccountInfo(AccountType.PREMIUM, password)
        else:
            self.info = AccountInfo(AccountType.STANDARD, password)

    def get_new_password(self, PESEL):
        # obviously placeholder
        return ('Password' + PESEL)

    def accepted_log_in(self, hashed_pass):
        local = hashlib.sha3_256(self.info.password.encode()).hexdigest()
        if hashed_pass == local:
            return True
        print('Unauthorized login attempt.')
        #print('local: ', local, ' remote: ', hashed_pass)
        return False


class StandardHandler(object):

    def __init__(self, accounts, accounts_lock, rates, rates_lock):
        self.accounts = accounts
        self.accounts_lock = accounts_lock
        self.rates = rates
        self.rates_lock = rates_lock

    def getBalance(self, PESEL, hash):
        print('Getting balance for ', PESEL)

        accounts_lock.acquire()

        if PESEL in accounts:
            account = accounts[PESEL]
            if account.accepted_log_in(hash):
                accounts_lock.release()
                return account.balance
            else:
                accounts_lock.release()
                raise FailedLoginException("Wrong password.")
        else:
            accounts_lock.release()
            raise FailedLoginException("No account with this PESEL.")


class PremiumHandler(StandardHandler):

    def getCredit(self, PESEL, hash, months, value, currency):
        print('Getting credit for ', PESEL, months, value)

        accounts_lock.acquire()

        if PESEL not in accounts:
            accounts_lock.release()
            raise FailedLoginException("No account with this PESEL.")

        account = accounts[PESEL]

        if not account.accepted_log_in(hash):
            accounts_lock.release()
            raise FailedLoginException("Wrong password.")

        if account.info.type != AccountType.PREMIUM:
            accounts_lock.release()
            raise FailedOperationException("Account is not premium, can't get loans!")

        accounts_lock.release()

        currency_rates_lock.acquire()

        currency_name = CurrencyName._VALUES_TO_NAMES[currency]
        if currency_name in currency_rates:
            rate = currency_rates[currency_name]
        else:
            currency_rates_lock.release()
            raise FailedOperationException("Currency not supported")
        currency_rates_lock.release()
        print('rate', rate, 'for', currency_name)
        # should be done better for floating point precission
        value_currency = (100 * value.dollars + value.cents)
        value_tmp = value_currency / rate

        value_tmp = math.ceil(value_tmp)

        # save credit to account balance
        accounts_lock.acquire()
        accounts[PESEL].balance.dollars += int((value_tmp - (value_tmp % 100)) / 100)
        accounts[PESEL].balance.cents += int(value_tmp % 100)
        accounts_lock.release()
        value_in_native = MonetaryType(int((value_tmp - (value_tmp % 100)) / 100), int(value_tmp % 100))
        value_currency = MonetaryType(int((value_currency - (value_currency % 100)) / 100),
                                      int(value_currency % 100))

        return {currency: value_currency, CurrencyName.PLN: value_in_native}


def runCurrencyHandler(currency_rates, currency_rates_lock):
    currency_rates_lock.acquire()
    currencies = [currency for currency in currency_rates]
    currency_rates_lock.release()

    rates = subscribe(currencies)
    for rate in rates:
        currency_rates_lock.acquire()
        currency_rates[rate[0]] = rate[1]
        currency_rates_lock.release()
        print(rate)


def runManageHandler(accounts, accounts_lock, port):
    handler = ManageHandler(accounts, accounts_lock)
    processor = ManageService.Processor(handler)
    transport = TSocket.TServerSocket(host='127.0.0.1', port=port)
    tfactory = TTransport.TBufferedTransportFactory()
    pfactory = TBinaryProtocolFactory()

    server = TServer.TThreadedServer(processor, transport, tfactory, pfactory)

    print('Starting manage server.')
    server.serve()
    print('Manage server done.')


def runPremiumHandler(currency_rates, currency_rates_lock, accounts, accounts_lock, port):
    handler = PremiumHandler(accounts, accounts_lock, currency_rates, currency_rates_lock)
    processor = PremiumService.Processor(handler)
    transport = TSocket.TServerSocket(host='127.0.0.1', port=port)
    tfactory = TTransport.TBufferedTransportFactory()
    pfactory = TBinaryProtocolFactory()

    server = TServer.TThreadedServer(processor, transport, tfactory, pfactory)

    print('Starting premium server.')
    server.serve()
    print('Manage server done.')


def runStandardHandler(currency_rates, currency_rates_lock, accounts, accounts_lock, port):
    handler = StandardHandler(accounts, accounts_lock, currency_rates, currency_rates_lock)
    processor = PremiumService.Processor(handler)
    transport = TSocket.TServerSocket(host='127.0.0.1', port=port)
    tfactory = TTransport.TBufferedTransportFactory()
    pfactory = TBinaryProtocolFactory()

    server = TServer.TThreadedServer(processor, transport, tfactory, pfactory)

    print('Starting standard server.')
    server.serve()
    print('Manage server done.')


def runAccountsHandler(currency_rates, currency_rates_lock, accounts, accounts_lock, port):
    standard_handler = StandardHandler(accounts, accounts_lock, currency_rates, currency_rates_lock)
    premium_handler = PremiumHandler(accounts, accounts_lock, currency_rates, currency_rates_lock)
    processor = TMultiplexedProcessor()
    processor.registerProcessor("PremiumService", PremiumService.Processor(premium_handler))
    processor.registerProcessor("StandardService", StandardService.Processor(standard_handler))

    transport = TSocket.TServerSocket(host='127.0.0.1', port=port)
    server = TServer.TThreadedServer(processor, transport)

    print('Starting accounts server.')
    server.serve()


if __name__ == '__main__':

    if len(sys.argv) < 3:
        print('Not enough arguments!')
        sys.exit(1)

    manage_port = int(sys.argv[1])
    accounts_port = manage_port + 1

    print('Bank server will accept clients on port: ', manage_port,
          ' and manage their accounts on port: ', accounts_port)

    currency_rates = {}

    for currency in sys.argv[2:]:
        if currency in dir(CurrencyName):
            print(currency, 'will be available in this bank.')
            currency_rates[currency] = 1.0
        else:
            print(currency, 'is not avaliable!')

    print(currency_rates)

    # for now maybe
    accounts = {}

    accounts_lock = threading.Lock()
    currency_rates_lock = threading.Lock()

    manage_thread = threading.Thread(target=runManageHandler,
                                     args=(accounts, accounts_lock, manage_port), daemon=True)

    currency_rates_thread = threading.Thread(target=runCurrencyHandler,
                                             args=(currency_rates, currency_rates_lock), daemon=True)

    #standard_thread = threading.Thread(target=runPremiumHandler,
    #                                   args=(currency_rates, currency_rates_lock, accounts, accounts_lock, accounts_port))
    #premium_thread = threading.Thread(target=runStandardHandler,
    #                                 args=(currency_rates, currency_rates_lock, accounts, accounts_lock, accounts_port))

    accounts_thread = threading.Thread(target=runAccountsHandler,
                                       args=(currency_rates, currency_rates_lock, accounts, accounts_lock, accounts_port),
                                       daemon=True)



    print('Starting manage thread.')
    manage_thread.start()
    print('Starting currencies thread.')
    currency_rates_thread.start()
    #print('Starting standard thread.')
    #standard_thread.start()
    #print('Starting premium thread.')
    #premium_thread.start()

    print('Starting accounts thread')
    accounts_thread.start()

    try:
        while True:
            time.sleep(_ONE_HOUR_IN_SECONDS)
    except KeyboardInterrupt:
        print('Done.')

