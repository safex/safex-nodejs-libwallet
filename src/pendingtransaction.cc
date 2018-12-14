#include "pendingtransaction.h"

#include <cmath>

#include <wallet_api.h>
#include "wallettasks.h"
#include "walletlog.h"

using namespace v8;

namespace exawallet {

Nan::Persistent<Function> PendingTransaction::constructor;

PendingTransaction::~PendingTransaction() {
    ENTER_FUNC();
    if (transaction) {
        delete transaction;
    }
    EXIT_FUNC();
}

NAN_MODULE_INIT(PendingTransaction::Init) {
    ENTER_FUNC();
    struct FunctionRegisterInfo {
        const char* name;
        Nan::FunctionCallback func;
    };

    static std::vector<FunctionRegisterInfo> functions = {
        {"commit", Commit},
        {"amount", Amount},
        {"dust", Dust},
        {"fee", Fee},
        {"transactionsIds", TransactionsIds},
        {"transactionsCount", TransactionsCount}
    };

    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(PendingTransaction::New);
    tpl->SetClassName(Nan::New("PendingTransaction").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(functions.size());

    for (const auto& info: functions) {
        Nan::SetPrototypeMethod(tpl, info.name, info.func);
    }

    constructor.Reset(tpl->GetFunction());
    EXIT_FUNC();
}

NAN_METHOD(PendingTransaction::New) {

    ENTER_FUNC();
  if (info.IsConstructCall()) {
    PendingTransaction* obj = new PendingTransaction(nullptr);
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    const int argc = 0;
    Local<Value> argv[1] = { Nan::Null() };
    Local<Function> cons = Nan::New(constructor);
    Local<Context> context = Nan::GetCurrentContext();
    Local<Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();
    info.GetReturnValue().Set(instance);
    EXIT_FUNC();
  }
}

Local<Object> PendingTransaction::NewInstance(SafexNativePendingTransaction* tx) {
    ENTER_FUNC();
    const unsigned argc = 0;
    Local<Value> argv[1] = { Nan::Null() };
    Local<Function> cons = Nan::New(constructor);
    Local<Context> context = Nan::GetCurrentContext();
    Local<Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();

    PendingTransaction* t = new PendingTransaction(tx);
    t->Wrap(instance);
    EXIT_FUNC();
    return instance;
}

NAN_METHOD(PendingTransaction::Commit) {
    ENTER_FUNC();
    PendingTransaction* obj = ObjectWrap::Unwrap<PendingTransaction>(info.Holder());

    CommitTransactionTask* task = new CommitTransactionTask(obj->transaction);
    auto promise = task->Enqueue();
    info.GetReturnValue().Set(promise);
    EXIT_FUNC();
}

NAN_METHOD(PendingTransaction::Amount) {
    ENTER_FUNC();
    PendingTransaction* obj = ObjectWrap::Unwrap<PendingTransaction>(info.Holder());
    auto amount = std::to_string(obj->transaction->amount());

    info.GetReturnValue().Set(Nan::New(amount.c_str()).ToLocalChecked());
    EXIT_FUNC();
}

NAN_METHOD(PendingTransaction::Dust) {
    PendingTransaction* obj = ObjectWrap::Unwrap<PendingTransaction>(info.Holder());
    auto dust = std::to_string(obj->transaction->dust());

    info.GetReturnValue().Set(Nan::New(dust.c_str()).ToLocalChecked());
    EXIT_FUNC();
}

NAN_METHOD(PendingTransaction::Fee) {
    ENTER_FUNC();
    PendingTransaction* obj = ObjectWrap::Unwrap<PendingTransaction>(info.Holder());
    auto fee = std::to_string(obj->transaction->fee());

    info.GetReturnValue().Set(Nan::New(fee.c_str()).ToLocalChecked());
    EXIT_FUNC();
}

NAN_METHOD(PendingTransaction::TransactionsIds) {
    ENTER_FUNC();
    PendingTransaction* obj = ObjectWrap::Unwrap<PendingTransaction>(info.Holder());

    auto transactions = obj->transaction->txid();
    auto result = Nan::New<Array>(transactions.size());

    for (size_t i = 0; i < transactions.size(); ++i) {
        auto id = Nan::New(transactions[i].c_str()).ToLocalChecked();
        if (result->Set(Nan::GetCurrentContext(), i, id).IsNothing()) {
            Nan::ThrowError("Couldn't make transactions list: unknown error");
            return;
        }
    }

    info.GetReturnValue().Set(result);
    EXIT_FUNC();
}

NAN_METHOD(PendingTransaction::TransactionsCount) {
    ENTER_FUNC();
    PendingTransaction* obj = ObjectWrap::Unwrap<PendingTransaction>(info.Holder());
    info.GetReturnValue().Set(Nan::New((uint32_t)obj->transaction->txCount()));
    EXIT_FUNC();
}



}
