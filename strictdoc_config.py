from strictdoc.core.project_config import ProjectConfig


def create_config() -> ProjectConfig:
    return ProjectConfig(
        project_title="Car Central Computer",
        project_features=[
            "TABLE_SCREEN",
            "TRACEABILITY_SCREEN",
            "DEEP_TRACEABILITY_SCREEN",
            "SEARCH",
            "TRACEABILITY_MATRIX_SCREEN",
            "REQUIREMENT_TO_SOURCE_TRACEABILITY",
            "PROJECT_STATISTICS_SCREEN",
            "REQIF",
        ],
        include_doc_paths=[
            "/docs/requirements/**.sdoc",
        ],
        include_source_paths=[
            "/src/**",
            "/tests/**",
        ],
    )
