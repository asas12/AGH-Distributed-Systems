namespace java bank
namespace py bank

exception FailedLoginException{
    1: string message
}
exception FailedToCreateException{
    1: string message
}
exception FailedOperationException{
    1: string message
}

service ManageService
{
        AccountInfo getAccount(1: ClientData client_data)
        throws (1: FailedToCreateException e),
}

service StandardService
{
        MonetaryType getBalance(1:string PESEL, 2:string hash)
		throws (1: FailedLoginException e),
}

service PremiumService extends StandardService
{
        map<CurrencyName, MonetaryType> getCredit(1:string PESEL, 2:string hash, 3:i32 months, 4:MonetaryType value, 5:CurrencyName currency)
		throws (1: FailedLoginException fle, 2: FailedOperationException e),
}

enum AccountType{
    STANDARD,
    PREMIUM
}

struct AccountInfo{
    1: AccountType type,
    2: string password
}

struct ClientData{
    1: string first_name,
    2: string last_name,
    3: string PESEL,
    4: i32 income
}

enum CurrencyName{
    PLN,
    EUR,
    USD,
    GBP,
    CHF
}

struct MonetaryType{
    1:i32 dollars,
    2:i32 cents
}


