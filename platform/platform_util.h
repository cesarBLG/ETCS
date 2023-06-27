 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <functional>
#include <optional>

#ifndef NO_THREADS
#define THREAD_LOCAL_DEF thread_local
#else
#define THREAD_LOCAL_DEF
#endif

namespace PlatformUtil
{
	template <typename T>
	struct CallbackType
	{
		CallbackType() = delete;
		typedef std::function<void(T&&)> type;
	};

	template <>
	struct CallbackType<void>
	{
		CallbackType() = delete;
		typedef std::function<void()> type;
	};

	class NoCopy
	{
	public:
		NoCopy(const NoCopy&) = delete;
		NoCopy& operator=(const NoCopy&) = delete;
		NoCopy() = default;
	};

	template <typename T>
	class Promise;
	template <typename T>
	class Fulfiller;
	class PromiseFactory;
	class DeferredFulfillment;

	template <typename T>
	class PromisePart
	{
		friend class PromiseFactory;
		friend class Promise<T>;
		friend class Fulfiller<T>;

		Fulfiller<T>* fulfiller;
	};

	class TypeErasedFulfiller
	{
		friend class DeferredFulfillment;

		virtual void execute_callback(bool defer) = 0;
	public:
		virtual ~TypeErasedFulfiller() = default;
	};

	template <typename T>
	class FulfillerTypeEraser final : public TypeErasedFulfiller {
		friend class Fulfiller<T>;

		Fulfiller<T> fulfiller;

		void execute_callback(bool defer) override { fulfiller.execute_callback(defer); }
	public:
		FulfillerTypeEraser(Fulfiller<T> &&f) : fulfiller(std::move(f)) {}
	};

	class DeferredFulfillment
	{
	public:
		DeferredFulfillment() = delete;

		static THREAD_LOCAL_DEF std::vector<std::unique_ptr<TypeErasedFulfiller>>* list;

		static bool execute() {
			if (list->empty())
				return false;

			auto tmp = std::move(*list);
			list->clear();
			for (auto &f : tmp)
				f->execute_callback(false);

			return true;
		}
	};

	class FuncFulfiller final : public TypeErasedFulfiller
	{
		template <typename T> friend class Promise;
		std::function<void()> callback;

		void execute_callback(bool defer) override {
			if (!defer)
				callback();
			else
				DeferredFulfillment::list->push_back(std::make_unique<FuncFulfiller>(std::move(*this)));
		}

		struct CreateTicket { };

	public:
		FuncFulfiller(std::function<void()> &&func, CreateTicket t) : callback(std::move(func)) {}
		FuncFulfiller(const std::function<void()> &func, CreateTicket t) : callback(func) {}
	};

	template <typename T>
	class Fulfiller
	{
		friend class PromiseFactory;
		friend class Promise<T>;
		friend class FulfillerTypeEraser<T>;

		PromisePart<T>* promise;
		std::optional<T> value;
		typename CallbackType<T>::type callback;
		std::function<void()> cancel_callback;
		FulfillerTypeEraser<T>* unmanaged;

		void execute_callback(bool defer) {
			if (defer) {
				if (unmanaged) {
					DeferredFulfillment::list->push_back(std::unique_ptr<FulfillerTypeEraser<T>>(unmanaged));
					unmanaged = nullptr;
				} else {
					DeferredFulfillment::list->push_back(std::make_unique<FulfillerTypeEraser<T>>(std::move(*this)));
				}
				return;
			}

			if (value)
				callback(std::move(*value));
			else
				cancel_callback();

			if (promise)
				promise->fulfiller = nullptr;
			promise = nullptr;
			callback = nullptr;
			cancel_callback = nullptr;
			if (unmanaged)
				delete unmanaged;
		}

	public:
		Fulfiller() {
			promise = nullptr;
			unmanaged = nullptr;
		}
		Fulfiller(Fulfiller &&other) {
			promise = nullptr;
			unmanaged = nullptr;
			*this = std::move(other);
		}
		Fulfiller& operator=(Fulfiller &&other) {
			if (promise)
				promise->fulfiller = nullptr;

			promise = other.promise;
			if (promise)
				promise->fulfiller = this;

			value = std::move(other.value);
			callback = std::move(other.callback);
			cancel_callback = std::move(other.cancel_callback);

			other.promise = nullptr;
			other.callback = nullptr;
			other.cancel_callback = nullptr;

			return *this;
		}
		~Fulfiller() {
			if (promise) {
				if (value) {
					auto e = new FulfillerTypeEraser<T>(std::move(*this));
					e->fulfiller.unmanaged = e;
				} else {
					promise->fulfiller = nullptr;
				}
			}
			if (!value && cancel_callback)
				execute_callback(true);
		}

		bool is_pending() const {
			if (value)
				return false;
			return callback != nullptr || promise != nullptr;
		}

		void fulfill(T&& arg, bool defer = true) {
			value = std::move(arg);
			if (callback)
				execute_callback(defer);
		}

		void fulfill(const T& arg, bool defer = true) {
			value = arg;
			if (callback)
				execute_callback(defer);
		}
	};

	template <>
	class Fulfiller<void>
	{
		friend class PromiseFactory;
		friend class Promise<void>;
		friend class FulfillerTypeEraser<void>;

		PromisePart<void>* promise;
		bool value;
		typename CallbackType<void>::type callback;
		std::function<void()> cancel_callback;
		FulfillerTypeEraser<void>* unmanaged;

		void execute_callback(bool defer) {
			if (defer) {
				if (unmanaged) {
					DeferredFulfillment::list->push_back(std::unique_ptr<FulfillerTypeEraser<void>>(unmanaged));
					unmanaged = nullptr;
				} else {
					DeferredFulfillment::list->push_back(std::make_unique<FulfillerTypeEraser<void>>(std::move(*this)));
				}
				return;
			}

			if (value) {
				if (callback)
					callback();
			} else {
				if (cancel_callback)
					cancel_callback();
			}

			if (promise)
				promise->fulfiller = nullptr;
			promise = nullptr;
			callback = nullptr;
			cancel_callback = nullptr;
			if (unmanaged)
				delete unmanaged;
		}

	public:
		Fulfiller() {
			promise = nullptr;
			value = false;
			unmanaged = nullptr;
		}
		Fulfiller(Fulfiller &&other) {
			promise = nullptr;
			unmanaged = nullptr;
			*this = std::move(other);
		}
		Fulfiller& operator=(Fulfiller &&other) {
			if (promise)
				promise->fulfiller = nullptr;

			promise = other.promise;
			if (promise)
				promise->fulfiller = this;

			value = other.value;
			callback = std::move(other.callback);
			cancel_callback = std::move(other.cancel_callback);

			other.promise = nullptr;
			other.callback = nullptr;
			other.cancel_callback = nullptr;

			return *this;
		}
		~Fulfiller() {
			if (promise) {
				if (value) {
					auto e = new FulfillerTypeEraser<void>(std::move(*this));
					e->fulfiller.unmanaged = e;
				} else {
					promise->fulfiller = nullptr;
				}
			}
			if (!value && cancel_callback)
				execute_callback(true);
		}

		bool is_pending() const {
			if (value)
				return false;
			return callback != nullptr || promise != nullptr;
		}

		void fulfill(bool defer = true) {
			value = true;
			if (callback)
				execute_callback(defer);
		}
	};

	template <typename T>
	class Promise
	{
		friend class PromiseFactory;
		friend class Fulfiller<T>;

		PromisePart<T> p;

		void cancel() {
			if (p.fulfiller) {
				p.fulfiller->callback = nullptr;
				p.fulfiller->cancel_callback = nullptr;
			}
		}

	public:
		Promise() {
			p.fulfiller = nullptr;
		}
		Promise(Promise &&other) {
			p.fulfiller = nullptr;
			*this = std::move(other);
		}
		Promise& operator=(Promise &&other) {
			cancel();
			detach();

			p.fulfiller = other.p.fulfiller;
			if (p.fulfiller)
				p.fulfiller->promise = &p;

			other.p.fulfiller = nullptr;

			return *this;
		}
		~Promise() {
			cancel();
			detach();
		}

		void detach() {
			if (p.fulfiller) {
				p.fulfiller->promise = nullptr;
				if (p.fulfiller->unmanaged)
					delete p.fulfiller->unmanaged;
			}
			p.fulfiller = nullptr;
		}

		Promise<T>&& then(const typename CallbackType<T>::type &func) {
			if (p.fulfiller) {
				p.fulfiller->callback = func;
				if (p.fulfiller->value)
					p.fulfiller->execute_callback(true);
			}
			return std::move(*this);
		}

		Promise<T>&& then(typename CallbackType<T>::type &&func) {
			if (p.fulfiller) {
				p.fulfiller->callback = std::move(func);
				if (p.fulfiller->value)
					p.fulfiller->execute_callback(true);
			}
			return std::move(*this);
		}

		Promise<T>&& otherwise(const std::function<void()> &func) {
			if (p.fulfiller)
				p.fulfiller->cancel_callback = func;
			else
				DeferredFulfillment::list->push_back(std::make_unique<FuncFulfiller>(func, FuncFulfiller::CreateTicket()));
			return std::move(*this);
		}

		Promise<T>&& otherwise(std::function<void()> &&func) {
			if (p.fulfiller)
				p.fulfiller->cancel_callback = std::move(func);
			else
				DeferredFulfillment::list->push_back(std::make_unique<FuncFulfiller>(std::move(func), FuncFulfiller::CreateTicket()));
			return std::move(*this);
		}
	};

	class TypeErasedPromise {
	public:
		virtual ~TypeErasedPromise() = default;
		virtual void detach() = 0;
	};

	template <typename T>
	class PromiseTypeEraser final : public TypeErasedPromise {
		Promise<T> promise;

	public:
		PromiseTypeEraser(Promise<T> &&p) : promise(std::move(p)) {}
		void detach() override { promise.detach(); }
	};

	class PromiseFactory
	{
		PromiseFactory() = delete;

	public:
		template <typename T>
		static std::pair<Promise<T>, Fulfiller<T>> create() {
			Promise<T> promise;
			Fulfiller<T> fulfiller;
			promise.p.fulfiller = &fulfiller;
			fulfiller.promise = &promise.p;
			return std::make_pair(std::move(promise), std::move(fulfiller));
		}
	};

	template <typename T>
	class FulfillerBufferedQueue
	{
		std::vector<Fulfiller<T>> fulfiller_queue;
		std::vector<T> data_queue;
		bool defer;

	public:
		FulfillerBufferedQueue(bool defer = true) : defer(defer) {}

		size_t pending_fulfillers() {
			fulfiller_queue.erase(std::remove_if(fulfiller_queue.begin(), fulfiller_queue.end(), [](const auto &entry) { return !entry.is_pending(); }), fulfiller_queue.end());
			return fulfiller_queue.size();
		}

		size_t pending_packets() {
			return data_queue.size();
		}

		void push_data(const T& packet) {
			auto it = fulfiller_queue.begin();
			while (it != fulfiller_queue.end() && !it->is_pending())
				++it;
			if (it != fulfiller_queue.end()) {
				Fulfiller<T> f = std::move(*it);
				fulfiller_queue.erase(it);
				f.fulfill(packet, defer);
			} else {
				data_queue.push_back(packet);
			}
		}

		void push_data(T&& packet) {
			auto it = fulfiller_queue.begin();
			while (it != fulfiller_queue.end() && !it->is_pending())
				++it;
			if (it != fulfiller_queue.end()) {
				Fulfiller<T> f = std::move(*it);
				fulfiller_queue.erase(it);
				f.fulfill(std::move(packet), defer);
			} else {
				data_queue.push_back(std::move(packet));
			}
		}

		void add(Fulfiller<T> &&f) {
			if (!f.is_pending())
				return;
			if (!data_queue.empty()) {
				auto it = data_queue.begin();
				T packet = std::move(*it);
				data_queue.erase(it);
				f.fulfill(std::move(packet), defer);
			} else {
				fulfiller_queue.push_back(std::move(f));
			}
		}

		Promise<T> create_and_add() {
			std::pair<Promise<T>, Fulfiller<T>> pair = PromiseFactory::create<T>();
			add(std::move(pair.second));
			return std::move(pair.first);
		}
	};

	template <typename T>
	class FulfillerList
	{
		std::vector<Fulfiller<T>> list;

	public:
		size_t pending() {
			list.erase(std::remove_if(list.begin(), list.end(), [](const auto &entry) { return !entry.is_pending(); }), list.end());
			return list.size();
		}

		void add(Fulfiller<T> &&f) {
			list.push_back(std::move(f));
		}

		void fulfill_one(const T& arg, bool defer = true) {
			std::vector<Fulfiller<T>> tmp = std::move(list);
			list.clear();

			auto it = tmp.begin();
			while (it != tmp.end() && !it->is_pending())
				++it;
			if (it != tmp.end()) {
				tmp.begin()->fulfill(arg, defer);
				auto tmp_next = std::next(it);
				if (tmp_next != tmp.end())
					list.insert(list.begin(), std::make_move_iterator(tmp_next), std::make_move_iterator(tmp.end()));
			}
		}

		void fulfill_one(T&& arg, bool defer = true) {
			std::vector<Fulfiller<T>> tmp = std::move(list);
			list.clear();

			auto it = tmp.begin();
			while (it != tmp.end() && !it->is_pending())
				++it;
			if (it != tmp.end()) {
				tmp.begin()->fulfill(std::move(arg), defer);
				auto tmp_next = std::next(it);
				if (tmp_next != tmp.end())
					list.insert(list.begin(), std::make_move_iterator(tmp_next), std::make_move_iterator(tmp.end()));
			}
		}

		void fulfill_all(const T& arg, bool defer = true) {
			std::vector<Fulfiller<T>> tmp = std::move(list);
			list.clear();
			for (PlatformUtil::Fulfiller<T> &f : tmp)
				f.fulfill(arg, defer);
		}

		void fulfill_all(T&& arg, bool defer = true) {
			std::vector<Fulfiller<T>> tmp = std::move(list);
			list.clear();
			if (tmp.size() == 1)
				tmp.begin()->fulfill(std::move(arg), defer);
			else if (tmp.size() > 1)
				for (PlatformUtil::Fulfiller<T> &f : tmp)
					f.fulfill(arg, defer);
		}

		Promise<T> create_and_add() {
			std::pair<Promise<T>, Fulfiller<T>> pair = PromiseFactory::create<T>();
			list.push_back(std::move(pair.second));
			return std::move(pair.first);
		}
	};

	template <>
	class FulfillerList<void>
	{
		std::vector<Fulfiller<void>> list;

	public:
		size_t pending() {
			list.erase(std::remove_if(list.begin(), list.end(), [](const auto &entry) { return !entry.is_pending(); }), list.end());
			return list.size();
		}

		void add(Fulfiller<void> &&f) {
			list.push_back(std::move(f));
		}

		void fulfill_one(bool defer = true) {
			std::vector<Fulfiller<void>> tmp = std::move(list);
			list.clear();

			auto it = tmp.begin();
			while (it != tmp.end() && !it->is_pending())
				++it;
			if (it != tmp.end()) {
				tmp.begin()->fulfill(defer);
				auto tmp_next = std::next(it);
				if (tmp_next != tmp.end())
					list.insert(list.begin(), std::make_move_iterator(tmp_next), std::make_move_iterator(tmp.end()));
			}
		}

		void fulfill_all(bool defer = true) {
			std::vector<Fulfiller<void>> tmp = std::move(list);
			list.clear();
			for (PlatformUtil::Fulfiller<void> &f : tmp)
				f.fulfill(defer);
		}

		Promise<void> create_and_add() {
			std::pair<Promise<void>, Fulfiller<void>> pair = PromiseFactory::create<void>();
			list.push_back(std::move(pair.second));
			return std::move(pair.first);
		}
	};
}
