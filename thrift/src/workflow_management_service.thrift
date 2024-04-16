namespace cpp hive.workflow

service workflow_management_service {
  // Register a workflow by upload a WDL json file
  void workflow_register(1: string workflow),

  // Deregister a workflow
  void workflow_deregister(1: string workflow_name),
}