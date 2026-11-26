#pragma once

#include <vob/aoe/data/DependencyCollectingVisitorContext.h>
#include <vob/aoe/data/FilesystemUtils.h>
#include <vob/aoe/data/FilesystemVisitorContext.h>
#include <vob/aoe/data/SoftPtr.h>

#include <vob/misc/visitor/json_reader.h>

#include <memory>
#include <string>


namespace vob::misvi
{
	template <typename TData>
	bool accept(
		pmr::json_reader<aoedt::FilesystemVisitorContext>& a_visitor
		, std::shared_ptr<TData const>& a_data)
	{
		std::string rawPath;
		a_visitor.visit(rawPath);

		auto const& context = a_visitor.get_context();
		auto const path = aoedt::FilesystemUtils::normalize(rawPath, context.getBasePath());
		auto const& indexer = context.getIndexer();
		a_data = context.getMultiDatabase().find<TData>(indexer.getRuntimeId(path));
		return true;
	}

	template <typename TData>
	bool accept(
		pmr::json_reader<aoedt::FilesystemVisitorContext>& a_visitor
		, aoedt::SoftPtr<TData>& a_data)
	{
		std::string rawPath;
		a_visitor.visit(rawPath);

		auto const& context = a_visitor.get_context();
		auto const path = aoedt::FilesystemUtils::normalize(rawPath, context.getBasePath());
		a_data = aoedt::SoftPtr<TData>{ context.getIndexer().getRuntimeId(path) };
		return true;
	}

	template <typename TData>
	bool accept(
		pmr::json_reader<aoedt::DependencyCollectingVisitorContext>& a_visitor
		, std::shared_ptr<TData const>&)
	{
		std::string rawPath;
		a_visitor.visit(rawPath);

		auto& context = a_visitor.get_context();
		context.getMultiDependencyCollector().collectDependencies<TData>(
			aoedt::FilesystemUtils::normalize(rawPath, context.getBasePath()), context.getDependencies());
		return true;
	}

	template <typename TData>
	bool accept(
		pmr::json_reader<aoedt::DependencyCollectingVisitorContext>& a_visitor
		, aoedt::SoftPtr<TData>&)
	{
		std::string rawPath;
		a_visitor.visit(rawPath);

		auto& context = a_visitor.get_context();
		context.getMultiDependencyCollector().collectDependencies<TData>(
			aoedt::FilesystemUtils::normalize(rawPath, context.getBasePath()), context.getDependencies());
		return true;
	}
}
