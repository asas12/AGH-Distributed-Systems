import sys
sys.path.append('./gen')

from gen.bank import StandardService, PremiumService, ManageService
from gen.bank.ttypes import *

from thrift import Thrift
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol
from thrift.protocol.TMultiplexedProtocol import TMultiplexedProtocol

import hashlib


def run(manage_port, accounts_port):

    try:
        transport_manage = TSocket.TSocket('localhost', manage_port)

        # Buffering is critical. Raw sockets are very slow
        transport_manage = TTransport.TBufferedTransport(transport_manage)

        # Wrap in a protocol
        protocol_manage = TBinaryProtocol.TBinaryProtocol(transport_manage)

        # Create a client to use the protocol encoder
        client_manage = ManageService.Client(protocol_manage)

        # Connect!
        transport_manage.open()

        transport_accounts = TSocket.TSocket('localhost', accounts_port)
        transport_accounts = TTransport.TBufferedTransport(transport_accounts)

        standard_protocol = TMultiplexedProtocol(TBinaryProtocol.TBinaryProtocol(transport_accounts), "StandardService")
        premium_protocol = TMultiplexedProtocol(TBinaryProtocol.TBinaryProtocol(transport_accounts), "PremiumService")

        client_premium = PremiumService.Client(premium_protocol)
        client_standard = StandardService.Client(standard_protocol)

        transport_accounts.open()

        try:
            client_info = client_manage.getAccount(ClientData('K', 'K', '900', 10000))
            print(client_info)
        except FailedToCreateException:
            pass
        password = 'Password900'
        credit_info = client_premium.getCredit('900',
                                               hashlib.sha3_256(password.encode()).hexdigest(),
                                               10,
                                               MonetaryType(1000, 0),
                                               getattr(CurrencyName, 'EUR'))
        print(credit_info)

        shell_loop(client_manage, client_standard, client_premium)

        transport_manage.close()
        transport_accounts.close()

    except Thrift.TException as tx:
        print('%s' % (tx.message))


def shell_loop(client_manage, client_standard, client_premium):
    print('Hello! Type \'create\' to create a new account,'
          ' \'balance\' to gen an existing one\'s balance and'
          ' \'credit\' to apply for a credit')
    try:

        while True:
            s = input('>')
            if s == 'create':
                print('Creating.')
                pesel = input('Please give your PESEL.\n>')
                first_name = input('Please give your first name.\n>')
                last_name = input('Please give your last name.\n>')
                income = int(input('Please give your declared monthly income.\n>'))

                client_data = ClientData(first_name, last_name, pesel, income)
                try:
                    account_info = client_manage.getAccount(client_data)
                    print('New account: ', AccountType._VALUES_TO_NAMES[account_info.type], account_info.password)
                except FailedToCreateException as tx:
                    print(tx.message)
            elif s == 'balance':
                pesel = input('Please give your PESEL.\n>')
                password = input('Please give your password.\n>')
                hashed_password = hashlib.sha3_256(password.encode())
                try:
                    balance = client_standard.getBalance(pesel, hashed_password.hexdigest())

                    print(balance.dollars, '.', balance.cents, ' PLN', sep='')

                except FailedLoginException as tx:
                    print(tx.message)
            elif s == 'credit':
                pesel = input('Please give your PESEL.\n>')
                password = input('Please give your password.\n>')
                months = int(input('Please give for how many months would you like to loan.\n>'))
                currency = input('Please specify currency, allowed: EUR, USD, GBP, CHF,'
                                 ' although not every bank covers all of them!\n>')
                amount = input('Please give the amount you would like to loan.\n>')

                if '.' in amount:
                    amount = amount.split('.')
                    amount = MonetaryType(int(amount[0]), int(amount[1]))
                elif ',' in amount:
                    amount = amount.split(',')
                    amount = MonetaryType(int(amount[0]), int(amount[1]))
                else:
                    amount = MonetaryType(int(amount), 0)

                hashed_password = hashlib.sha3_256(password.encode())
                try:
                    credit = client_premium.getCredit(pesel, hashed_password.hexdigest(), months, amount, getattr(CurrencyName, currency))

                    print('You have been granted a credit: ')
                    for currency in credit:
                        print('\t', credit[currency].dollars, '.', credit[currency].cents, ' ', CurrencyName._VALUES_TO_NAMES[currency], sep='')

                except FailedLoginException as tx:
                    print(tx.message)
                except FailedOperationException as tx:
                    print(tx.message)

            else:
                print('Unknown command')
    except KeyboardInterrupt:
        print('Exiting.')


if __name__ == '__main__':

    if len(sys.argv) < 2:
        print('Not enough arguments!')
        sys.exit(1)

    manage_port = int(sys.argv[1])
    accounts_port = manage_port + 1

    run(manage_port, accounts_port)
