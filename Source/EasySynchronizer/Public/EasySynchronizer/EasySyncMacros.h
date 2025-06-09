#pragma once

#define EASYSYNC_GENERATED_CONDITION() \
	public: \
		struct FConditionDataType : public FEasySyncBaseConditionData \
		{ \
			enum \
			{ \
				DataPropertiesNum = 0 \
			};\
		}; \
	\
	public: \
		static TSharedPtr<FEasySyncConditionHandler> CheckWith() \
		{ \
			auto Data = MakeShared<FConditionDataType>(); \
			\
			return MakeShared<FEasySyncConditionHandler>(StaticClass(), MoveTemp(Data)); \
		} \
	\
	protected: \
		\
		virtual EEasySyncConditionStatus ConditionStatus(const FEasySyncBaseConditionData& Data) override \
		{ \
			const auto CastedData = CastConditionData<FConditionDataType>(Data); \
			if(!CastedData) return EEasySyncConditionStatus::Error; \
			\
			return ConditionStatusInternal(); \
		} \
	\
	private:\
		EEasySyncConditionStatus ConditionStatusInternal();


#define EASYSYNC_GENERATED_CONDITION_CUSTOMHASH_OneParam(PARAM1_TYPE, PARAM1_NAME) \
	public: \
		struct FConditionDataType : public FEasySyncBaseConditionData \
		{ \
			enum \
			{ \
				DataPropertiesNum = 1 \
			}; \
			\
			FConditionDataType() = default; \
			\
			FConditionDataType(PARAM1_TYPE In##PARAM1_NAME) : PARAM1_NAME(MoveTemp(In##PARAM1_NAME)) \
			{} \
			\
			PARAM1_TYPE PARAM1_NAME; \
			\
			uint32 GetHash() const override; \
		}; \
	\
	public: \
		static TSharedPtr<FEasySyncConditionHandler> CheckWith(const PARAM1_TYPE& PARAM1_NAME) \
		{ \
			auto Data = MakeShared<FConditionDataType>(); \
			Data->PARAM1_NAME = PARAM1_NAME; \
			\
			return MakeShared<FEasySyncConditionHandler>(StaticClass(), MoveTemp(Data)); \
		} \
	\
	protected: \
		\
		virtual EEasySyncConditionStatus ConditionStatus(const FEasySyncBaseConditionData& Data) override \
		{ \
			const auto CastedData = CastConditionData<FConditionDataType>(Data); \
			if(!CastedData) return EEasySyncConditionStatus::Error; \
			\
			return ConditionStatusInternal(CastedData->PARAM1_NAME); \
		} \
	\
	private:\
		EEasySyncConditionStatus ConditionStatusInternal(const PARAM1_TYPE& PARAM1_NAME);

