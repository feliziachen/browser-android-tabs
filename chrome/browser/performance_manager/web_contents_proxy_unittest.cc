// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/performance_manager/web_contents_proxy.h"

#include "base/run_loop.h"
#include "base/task/post_task.h"
#include "base/task/task_traits.h"
#include "base/test/bind_test_util.h"
#include "chrome/browser/performance_manager/graph/page_node_impl.h"
#include "chrome/browser/performance_manager/performance_manager_tab_helper.h"
#include "chrome/browser/performance_manager/performance_manager_test_harness.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"

namespace performance_manager {

class WebContentsProxyTest : public PerformanceManagerTestHarness {
 public:
  WebContentsProxyTest() {}
  ~WebContentsProxyTest() override {}

  void GetContentsViaProxy(bool delete_before_deref);

 private:
  DISALLOW_COPY_AND_ASSIGN(WebContentsProxyTest);
};

TEST_F(WebContentsProxyTest, EndToEnd) {
  std::unique_ptr<content::WebContents> contents = CreateTestWebContents();
  auto* helper = PerformanceManagerTabHelper::FromWebContents(contents.get());
  auto* page_node = helper->page_node();
  content::WebContents* proxy_contents = nullptr;

  auto deref_proxy = base::BindLambdaForTesting(
      [&proxy_contents](const base::WeakPtr<WebContentsProxy>& proxy,
                        base::RepeatingClosure quit_loop) {
        proxy_contents = nullptr;
        if (proxy)
          proxy_contents = proxy.get()->GetWebContents();
        quit_loop.Run();
      });

  // Bounce over to the PM sequence, retrieve the proxy, bounce back to the UI
  // thread, dereference it if possible, and save the returned contents. To be
  // fair, it's entirely valid to grab the weak pointer directly on the UI
  // thread, as the lifetime of the page node is managed their and the property
  // being accessed is thread safe. However, this test aims to simulate what
  // would happen with a policy message being posted from the graph.
  {
    base::RunLoop run_loop;
    PerformanceManager::GetInstance()->CallOnGraph(
        FROM_HERE, base::BindLambdaForTesting([&run_loop, &deref_proxy,
                                               page_node](Graph* graph) {
          base::PostTaskWithTraits(
              FROM_HERE, {content::BrowserThread::UI},
              base::BindOnce(deref_proxy, page_node->contents_proxy(),
                             run_loop.QuitClosure()));
        }));
    run_loop.Run();

    EXPECT_EQ(contents.get(), proxy_contents);
  }

  // Run the same test, but this time destroy the contents prior to
  // dereferencing the proxy.
  {
    base::RunLoop run_loop;
    PerformanceManager::GetInstance()->CallOnGraph(
        FROM_HERE,
        base::BindLambdaForTesting([&contents, &run_loop, &deref_proxy,
                                    page_node](Graph* graph) {
          base::PostTaskWithTraits(
              FROM_HERE, {content::BrowserThread::UI},
              base::BindLambdaForTesting([&contents]() { contents.reset(); }));
          base::PostTaskWithTraits(
              FROM_HERE, {content::BrowserThread::UI},
              base::BindOnce(deref_proxy, page_node->contents_proxy(),
                             run_loop.QuitClosure()));
        }));
    run_loop.Run();

    // The contents was destroyed on the UI thread prior to dereferencing the
    // proxy, so it should return nullptr.
    EXPECT_FALSE(proxy_contents);
  }
}

}  // namespace performance_manager
